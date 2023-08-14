#include "RequestHandler.hpp"
#include "../cgi/CgiResolver.hpp"
#include "../client/Client.hpp"
#include "../response/HttpResponseException.hpp"
#include "../response/Mime.hpp"
#include "../selector/Mediator.hpp"
#include "../utils/string.hpp"
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>
#include <unistd.h> // access

#include <dirent.h>

#include "../server/utils.hpp"

#include <vector>

RequestHandler::RequestHandler() : handled(false) {}

RequestHandler::RequestHandler(const RequestHandler &o)
    : response(o.response), request(o.request), servers(o.servers), fd(o.fd),
      handled(o.handled), list_dir(false), file(o.file), targetLoc(o.targetLoc),
      matchLocState(o.matchLocState) {}

RequestHandler::RequestHandler(HttpRequest &request,
                               std::vector<Server> &servers) {
    this->request = request;
    fd = -1;
    this->servers = servers;
}

RequestHandler &RequestHandler::operator=(const RequestHandler &o) {
    if (this == &o)
        return *this;

    this->response = o.response;
    this->request = o.request;
    this->servers = o.servers;
    this->fd = o.fd;
    this->handled = o.handled;
    this->file = o.file;
    this->targetLoc = o.targetLoc;
    this->matchLocState = o.matchLocState;

    return *this;
}

RequestHandler::~RequestHandler() {}

bool RequestHandler::isDirChecks(Client &client) {
    struct stat data;
    stat(file.c_str(), &data);
    if (!S_ISDIR(data.st_mode))
        return false;
    AContext *loader = this->matchLocState
                           ? dynamic_cast<AContext *>(&this->targetLoc)
                           : dynamic_cast<AContext *>(&client.getServer());
    if (loader->getAutoindex()) {
        this->list_dir = true;
        return true;
    } else {
        if (setIndexFile(loader->getIndex()))
            return false;
        this->file.clear();
    }
    return false;
}

bool RequestHandler::setIndexFile(const std::vector<std::string> &indexes) {
    forEachConst(std::vector<std::string>, indexes, itr) {
        std::string holder = this->file + "/" + *itr;
        if (access(holder.c_str(), F_OK | R_OK) == 0) {
            this->file = holder;
            return true;
        }
    }
    return false;
}

void RequestHandler::init(Client &client) {
    Server srv;

    this->list_dir = false;

    std::string hostHeader = request.getHeader("Host");
    srv = validServerName(hostHeader);
    client.setServer(srv);
    client.setMethod(request.getMethod());
    this->matchLocState =
        matchLocation(request.getEndpoint(), client.getServer());
    validMethod(client.getMethod(), client);
    if (client.getMethod() == "DELETE")
        return;

    checkConfAndAccess(client);

    if (matchLocState && !this->targetLoc.getRedirect().empty()) {
        setInitialized(false);
        throw HttpResponseException(307, targetLoc.getRedirect());
    }

    client.setContentLength(
        utils::string::toInt(request.getHeader("Content-Length")));

    if (isDirChecks(client)) {
        if (*(request.getEndpoint().rbegin()) != '/') {
            throw HttpResponseException(307, request.getEndpoint() + "/");
        }
        return;
    }

    std::string::size_type dot = file.find_last_of('.');
    if (dot != std::string::npos)
        this->extension = file.substr(dot);

    client.setChunkedRequest(request.isChunked());

    if (isCGI(this->extension)) {
        return;
    }

    if (request.isMultipartData()) {
        client.setFormData(true);
        client.setFormDataBoundary(request.getMultipartDataBoundary());
    }

    if (client.getFormDataStatus() && client.getChunkedRequestStatus()) {
        throw HttpResponseException(501);
    }
}

void RequestHandler::fileRequested(Client &client, Mediator &mediator) {
    if (this->file.empty())
        throw HttpResponseException(404);
    if (access(file.c_str(), F_OK) == -1)
        throw HttpResponseException(404);
    if (access(file.c_str(), R_OK == -1))
        throw HttpResponseException(403);
    struct stat data;
    stat(file.c_str(), &data);

    if (isCGI(this->extension)) {
        CGIResolver cgi(this->getCgiPathFromExtension(this->extension), file,
                        this->request, client);
        client.setCgiReadFd(cgi.getReadEnd());
        client.setCgiWriteFd(cgi.getWriteEnd());
        mediator.addReadCGI(cgi.getReadEnd());
        mediator.addWriteCGI(cgi.getWriteEnd());
        return;
    }

    off_t length = data.st_size;

    this->fd = open(file.c_str(), O_RDONLY);
    response.setStatuscode(200)
        ->setHeader("Content-Type", this->getFileMimeType(file))
        ->setHeader("Content-Length", utils::string::fromInt(length));
    client.storeResponse(this->getResponse());
    client.setFileFd(this->getFd());
}

void RequestHandler::handleGET(Client &client, Mediator &mediator) {

    if (request.getHeader("Connection") == "close")
        client.setConnectionClose(true);
    if (this->list_dir)
        listDirectory(client);
    else
        fileRequested(client, mediator);
}

void RequestHandler::handlePOST(Client &client, Mediator &mediator) {
    if (request.getHeader("Connection") == "close")
        client.setConnectionClose(true);

    if (this->list_dir)
        listDirectory(client);
    else
        fileRequested(client, mediator);
}

void RequestHandler::DeleteFiles(const std::string &path,
                                 std::vector<std::string> &list) {
    struct stat buff;
    stat(path.c_str(), &buff);
    if (S_ISDIR(buff.st_mode)) {
        DIR *d = opendir(path.c_str());
        for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {
#if __linux__
            std::string s(de->d_name);
#elif __APPLE__
            std::string s(de->d_name, de->d_namlen);
#endif
            if (s != ".." && s != ".") {
                s = std::string(path) + "/" + s;
                DeleteFiles(s, list);
            }
            closedir(d);
        }
    }

    list.push_back(path);
}

void RequestHandler::handleDELETE(Client &client) {
    if (request.getHeader("Connection") == "close")
        client.setConnectionClose(true);
    // DELETE THE FILE
    std::string delete_me = targetLoc.getRoot() + this->request.getEndpoint();

    std::vector<std::string> list;
    DeleteFiles(delete_me, list);
    for (size_t i = 0; i < list.size(); i++) {
        if (access(list[i].c_str(), F_OK) != 0)
            throw HttpResponseException(404);
        if (access(list[i].c_str(), W_OK) != 0)
            throw HttpResponseException(403);
    }
    for (size_t i = 0; i < list.size(); i++)
        remove(list[i].c_str());

    throw HttpResponseException(204);
}

void RequestHandler::checkConfAndAccess(Client &client) {
    file = request.getEndpoint();

    if (this->matchLocState) {
        std::string LocationRoot = this->targetLoc.getRoot();
        if (LocationRoot.empty())
            LocationRoot = client.getServer().getRoot();
        file = LocationRoot + file;
    } else {
        file = client.getServer().getRoot() + file;
    }
}

void RequestHandler::fillContainer(std::string &container) {
    DIR *d = opendir(file.c_str());
    if (!d)
        throw HttpResponseException(403);
    for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {
        std::string item;
#if __linux__
        std::string s(de->d_name);
#elif __APPLE__
        std::string s(de->d_name, de->d_namlen);
#endif
        if (s == "." || s == "..")
            continue;
        container += "<li><a href='" + s + "'>" + s + "</a></li>";
    }
    closedir(d);
}

void RequestHandler::listDirectory(Client &client) {
    std::string container =
        "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>\
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
        <title>" +
        this->request.getEndpoint() +
        "</title></head><body><div><h2>Index of: " +
        this->request.getEndpoint() + "</h2><ul>";

    fillContainer(container);
    container += "</ul></div></body></html>";

    response.setStatuscode(200)
        ->setHeader("Content-Type", this->getFileMimeType(".html"))
        ->pushBody(container);
    client.storeResponse(this->getResponse());
    client.setFileFd(this->getFd());
}

bool RequestHandler::isCGI(const std::string &extension) {

    try {
        this->targetLoc.getCgiPath().at(extension);
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

const std::string &
RequestHandler::getCgiPathFromExtension(const std::string &extension) {
    return this->targetLoc.getCgiPath().find(extension)->second;
}

std::string RequestHandler::getResponse() { return response.build(); }

bool RequestHandler::matchLocation(const std::string &endpoint,
                                   const Server &serv) {
    std::vector<Location> locations = serv.getLocation();
    if (locations.empty()) {
        return false;
    }
    bool found = false;
    std::string holder = "";
    forEach(std::vector<Location>, locations, itr) {
        forEachConst(std::vector<std::string>, itr->getPrefix(), itr1) {
            std::vector<std::string> list = utils::split(endpoint, "/");
            for (size_t i = 0; i < list.size(); i++) {
                std::string tmp = "";
                for (std::vector<std::string>::iterator itr2 = list.begin();
                     itr2 != list.begin() + i + 1; itr2++) {
                    tmp += "/" + *itr2;
                }
                std::string prefix = *itr1;
                utils::strTrimV2(prefix, "/");
                prefix = "/" + prefix;
                if (tmp == prefix || prefix == "/") {
                    found = true;
                    holder = prefix;
                    this->targetLoc = *itr;
                    return true;
                }
            }
            if (list.empty() && *itr1 == "/") {
                this->targetLoc = *itr;
                return true;
            }
        }
    }

    return found;
}

Server &RequestHandler::validServerName(std::string serverName) {
    for (std::vector<Server>::iterator itr = servers.begin();
         itr != servers.end(); itr++) {
        std::vector<std::string> server_names = itr->getServerNames();
        for (std::vector<std::string>::iterator itr1 = server_names.begin();
             itr1 != server_names.end(); itr1++) {
            if (serverName == *itr1) {
                return *itr;
            }
        }
    }
    return *(servers.begin());
}

int RequestHandler::getFd() { return this->fd; }

void RequestHandler::setInitialized(bool handled) { this->handled = handled; }

bool RequestHandler::hasBeenInitialized() const { return handled; }

std::string
RequestHandler::getFileMimeType(const std::string &file_name) const {
    std::string extention = file_name.substr(file_name.find_last_of('.') + 1);
    return Mime::getInstance()->getMimeType(extention);
}

void RequestHandler::validMethod(const std::string &method, Client &c) {
    if (this->matchLocState) {
        if (!targetLoc.getAllowedMethods().at(method))
            throw HttpResponseException(405);
    } else {
        if (!c.getServer().getAllowedMethods().at(method)) {
            throw HttpResponseException(405);
        }
    }
}

Location &RequestHandler::getLocation() { return this->targetLoc; }
bool RequestHandler::matchedLocation() { return this->matchLocState; }
