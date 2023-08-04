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
      handled(o.handled), file(o.file), targetLoc(o.targetLoc),
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

void RequestHandler::init(Client &client) {
    Server srv;

    std::string hostHeader = request.getHeader("Host");
    srv = validServerName(hostHeader);
    client.setServer(srv);
    client.setMethod(request.getMethod());
    client.setContentLength(
        utils::string::toInt(request.getHeader("Content-Length")));
    if (request.getHeader("Transfer-Encoding") == "chunked") {
        client.setChunkedRequest(true);
    }
    // client setMaxBodySize();
}

void RequestHandler::fileRequested(Client &client, Mediator &mediator) {
    struct stat data;
    stat(file.c_str(), &data);

    std::string extension = file.substr(file.find_last_of('.'));

    if (checkForExtension(extension)) {

        CGIResolver cgi(this->getCgiPathFromExtension(extension), file,
                        this->request, client);
        client.setCgiFd(cgi.getReadEnd());
        mediator.addCGI(cgi.getReadEnd());
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
    (void)mediator;

    if (request.getHeader("Connection") == "close")
        client.setConnectionClose(true);

    checkConfAndAccess(client);
    validMethod("GET", client);

    struct stat data;
    stat(file.c_str(), &data);

    // check for the existence of the dir
    if (S_ISDIR(data.st_mode)) {
        listDirectory(client, mediator);
        return;
    } else {
        fileRequested(client, mediator);
    }
}

void RequestHandler::handlePOST(Client &client, Mediator &mediator) {
    checkConfAndAccess(client);
    validMethod("POST", client);

    struct stat data;
    stat(file.c_str(), &data);

    if (S_ISDIR(data.st_mode)) {
        this->responseFromIndexes(client, mediator);
    } else {
        fileRequested(client, mediator);
        return;
    }

    // client.storeResponse(this->getResponse());
    // client.setFileFd(this->getFd());

    // fileRequested(client, mediator);

    // if cgi meaning if the extension matches a cgi and we have to go through
    // it if (cgi()) {
    //   if (request.getMethod() == "POST") {
    //    pushStatusLine, headers (chunked transfer encoding) and \r\n\r\n
    //  }
    //  CGI cgi = CGI(path, ext);
    //  client.setCgiFd(pipe[0]);
    //  mediator.addCgi(pipe[0]);
    // } else {
    //  client.setBodyFd(file) // if the file exists etc
    // }
}

void RequestHandler::checkConfAndAccess(Client &client) {
    file = request.getEndpoint();
    std::cout << "end point: " << file << std::endl;

    this->matchLocState = matchLocation(file, client.getServer());
    if (this->matchLocState) {
        std::string LocationRoot = this->targetLoc.getRoot();
        if (LocationRoot.empty())
            LocationRoot = client.getServer().getRoot();
        file = "." + LocationRoot + file;
    } else {
        file = "." + client.getServer().getRoot() + file;
    }

    std::cout << "-->file: " << file << std::endl;
    std::cout << "-->function state: " << std::boolalpha << this->matchLocState
              << std::endl;
    if (access(file.c_str(), F_OK) == -1)
        throw HttpResponseException(404);
    if (access(file.c_str(), R_OK == -1))
        throw HttpResponseException(403);
}

void RequestHandler::createContainer(std::string &container,
                                     std::string::size_type &index) {
    std::fstream strm("./root/listDir.html", std::ios::in);
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

        std::cout << "file name: " << s << std::endl;

        if (DT_DIR == de->d_type && s == "..")
            item = std::string("\t\t\t<li style=\"list-style-image: "
                               "url('/images/arrow.png')\"><a href=\"") +
                   s + "\">" + s + "</a>" + "</li>\n";
        else if (DT_DIR == de->d_type) {
            item = std::string("\t\t\t<li style=\"list-style-image: "
                               "url('/images/folder.png')\"><a href=\"") +
                   s + "\">" + s + "</a>" + "</li>\n";
        } else {
            item = std::string("\t\t\t<li style=\"list-style-image: "
                               "url('/images/file.png')\"><a href=\"") +
                   s + "\">" + s + "</a>" + "</li>\n";
        }

        container.insert(index, item);
        index += item.length();
    }
}

void RequestHandler::listDirectory(Client &client, Mediator &mediator) {
    if (this->matchLocState) {
        if (this->targetLoc.getAutoindex()) {
            std::string container;
            std::string::size_type index = 0;

            createContainer(container, index);
            fillContainer(container, index);

            response.setStatuscode(200)
                ->setHeader("Content-Type", this->getFileMimeType(".html"))
                ->pushBody(container);
            client.storeResponse(this->getResponse());
            client.setFileFd(this->getFd());
            return;

        } else
            this->responseFromIndexes(client, mediator);
    } else
        throw HttpResponseException(404);
}

bool RequestHandler::checkForExtension(const std::string &extension) {
    try {
        this->targetLoc.getCgiPath().at(extension);
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

void RequestHandler::responseFromIndexes(Client &client, Mediator &mediator) {
    std::vector<std::string> indexes = this->targetLoc.getIndex();
    forEach(std::vector<std::string>, indexes, itr) {
        std::string holder = this->file + "/" + *itr;
        if (access(holder.c_str(), F_OK | R_OK) == 0) {
            this->file = holder;
            fileRequested(client, mediator);
            return;
        }
    }
    throw HttpResponseException(404);
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
                // list.pop_back();

                std::cout << "tmp: " << tmp << " | i=  " << i
                          << " | size: " << list.size() << std::endl;

                std::string prefix = *itr1;
                utils::strTrimV2(prefix, "/");
                prefix.insert(prefix.begin(), '/');
                std::cout << "prefix modified: " << prefix << std::endl;
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
        std::cout << "================ Location ==================="
                  << std::endl;
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
