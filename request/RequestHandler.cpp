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
    if (this->matchLocState == true) {
        if (this->targetLoc.getAutoindex() == true) {
            this->list_dir = true;
            return true;
        } else {
            if (setIndexFile(this->targetLoc.getIndex()))
                return false;
            this->file.clear();
        }
    } else {
        if (client.getServer().getAutoindex()) {
            this->list_dir = true;
            return true;
        } else {
            if (setIndexFile(client.getServer().getIndex()))
                return false;
            this->file.clear();
        }
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
    this->matchLocState = matchLocation(file, client.getServer());
    checkConfAndAccess(client);

    client.setContentLength(
        utils::string::toInt(request.getHeader("Content-Length")));

    if (isDirChecks(client))
        return;

    std::string::size_type dot = file.find_last_of('.');
    if (dot != std::string::npos)
        this->extension = file.substr(dot);

    if (request.getHeader("Transfer-Encoding") == "chunked") {
        client.setChunkedRequest(true);
    }

    if (checkForExtension(this->extension)) {
        return;
    }

    if (request.getHeader("Content-Type") != "") {
        std::string header_value = this->request.getHeader("Content-Type");
        std::string::size_type semicolon_pos = header_value.find(';');
        std::string content_type = header_value.substr(0, semicolon_pos);

        if (content_type == "multipart/form-data") {
            std::string::size_type boundary_pos = semicolon_pos + 1;
            if (content_type[boundary_pos] == ' ')
                boundary_pos++;
            boundary_pos += 10;
            std::string boundary = header_value.substr(boundary_pos);
            client.setFormData(true);
            client.setFormDataBoundary(boundary);
        }
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

    if (checkForExtension(this->extension)) {
        CGIResolver cgi(this->getCgiPathFromExtension(this->extension), file,
                        this->request, client);
        client.setCgiFd(cgi.getReadEnd());
        mediator.addCGI(cgi.getReadEnd());
        std::cout << cgi.getReadEnd() << std::endl;
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

    validMethod("GET", client);
    if (this->list_dir)
        listDirectory(client, mediator);
    else
        fileRequested(client, mediator);
}

void RequestHandler::handlePOST(Client &client, Mediator &mediator) {
    if (request.getHeader("Connection") == "close")
        client.setConnectionClose(true);

    validMethod("POST", client);
    if (this->list_dir)
        listDirectory(client, mediator);
    else
        fileRequested(client, mediator);
}

void RequestHandler::checkConfAndAccess(Client &client) {
    file = request.getEndpoint();

    if (this->matchLocState) {
        std::string LocationRoot = this->targetLoc.getRoot();
        if (LocationRoot.empty())
            LocationRoot = client.getServer().getRoot();
        file = "." + LocationRoot + file;
    } else {
        file = "." + client.getServer().getRoot() + file;
    }
}

void RequestHandler::createContainer(std::string &container,
                                     std::string::size_type &index) {
    std::fstream strm("./www/listDir.html", std::ios::in);
    if (!strm.is_open())
        throw HttpResponseException(500);

    char c;
    while (!strm.eof()) {
        strm.get(c);
        container.push_back(c);
    }
    strm.close();

    std::string s1("{content}");
    index = container.find(s1);
    if (index == std::string::npos) {
        throw HttpResponseException(500);
    }
    container.erase(index, s1.length());
}

void RequestHandler::fillContainer(std::string &container,
                                   std::string::size_type &index) {

    DIR *d = opendir(file.c_str());
    for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {
        std::string item;
        std::string s(de->d_name);

        if (DT_DIR == de->d_type && s == "..")
            item = std::string("\t\t\t<li ><a href=\"") + s + "\">" + s +
                   "</a>" + "</li>\n";
        else if (DT_DIR == de->d_type) {
            item = std::string("\t\t\t<li\"><a href=\"") + s + "\">" + s +
                   "</a>" + "</li>\n";
        } else {
            item = std::string("\t\t\t<li\"><a href=\"") + s + "\">" + s +
                   "</a>" + "</li>\n";
        }

        container.insert(index, item);
        index += item.length();
    }
}

void RequestHandler::listDirectory(Client &client, Mediator &mediator) {
    (void)mediator;
    std::string container;
    std::string::size_type index = 0;

    createContainer(container, index);
    fillContainer(container, index);

    response.setStatuscode(200)
        ->setHeader("Content-Type", this->getFileMimeType(".html"))
        ->pushBody(container);
    client.storeResponse(this->getResponse());
    client.setFileFd(this->getFd());
}

bool RequestHandler::checkForExtension(const std::string &extension) {

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

bool RequestHandler::matchLocation(std::string endpoint, const Server &serv) {

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
                prefix.insert(prefix.begin(), '/');
                if (tmp == prefix || prefix == "/") {
                    found = true;
                    holder = prefix;
                    this->targetLoc = *itr;
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

const Location &RequestHandler::getLocation() { return this->targetLoc; }
bool RequestHandler::matchedLocation() { return this->matchLocState; }