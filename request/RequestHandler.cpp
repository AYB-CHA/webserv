#include "RequestHandler.hpp"
#include "../response/HttpResponseException.hpp"
#include "../response/Mime.hpp"
#include "../utils/string.hpp"
#include "../client/Client.hpp"
#include "../selector/Mediator.hpp"
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>
#include <unistd.h> // access

#include <vector>

RequestHandler::RequestHandler() : handled(false) {}

RequestHandler::RequestHandler(const RequestHandler& o)
    : response(o.response), request(o.request),
    servers(o.servers), fd(o.fd), handled(o.handled)
{}

RequestHandler::RequestHandler(HttpRequest &request, std::vector<Server>& servers) {
    this->request = request;
    fd = -1;
    this->servers = servers;
}

void RequestHandler::init(Client& client) {
    Server srv;

    std::string hostHeader = request.getHeader("Host");
    srv = validServerName(hostHeader);
    client.setServer(srv);
    client.setMethod(request.getMethod());
    client.setContentLength(utils::string::toInt(request.getHeader("Content-Length")));
}

bool RequestHandler::handlePOST(Client &client, Mediator& mediator) {
    (void)client;
    (void)mediator;
    // if cgi meaning if the extension matches a cgi and we have to go through it
    // if (cgi()) {
    //   if (request.getMethod() == "POST") {
    //    pushStatusLine, headers (chunked transfer encoding) and \r\n\r\n
    //  }
    //  CGI cgi = CGI(path, ext);
    //  client.setCgiFd(pipe[0]);
    //  mediator.addCgi(pipe[0]);
    // } else {
    //  client.setBodyFd(file) // if the file exists etc
    // }
    return true;
}

void RequestHandler::handleGET(Client& client) {
    std::string file = request.getEndpoint();

    Location targetLoc = matchLocation(
        file,
        const_cast<std::vector<Location> &>(client.getServer().getLocation()));

    file = "./" + targetLoc.getRoot() + file;
    // file = "./types.txt";
    // std::cout << file << std::endl;
    if (access(file.c_str(), F_OK) == -1)
        throw HttpResponseException(404);
    if (access(file.c_str(), R_OK == -1))
        throw HttpResponseException(403);

    struct stat data;
    stat(file.c_str(), &data);
    off_t length = data.st_size;

    fd = open(file.c_str(), O_RDONLY);

    std::cout << "len: " << length << std::endl;
    // std::cout << "fd: " << fd << std::endl;
    response.setStatuscode(200)
        ->setHeader("Content-Type", this->getFileMimeType(file))
        ->setHeader("Content-Length", utils::string::fromInt(length));
    client.storeResponse(this->getResponse());
    client.setFileFd(this->getFd());
}

std::string RequestHandler::getResponse() { return response.build(); }

Location RequestHandler::matchLocation(std::string endpoint, std::vector<Location>& locations) {
    Location target;
    std::string holder = "";

    for (std::vector<Location>::iterator itr = locations.begin();
         itr != locations.end(); itr++) {
        std::vector<std::string> prefixs = itr->getPrefix();
        for (std::vector<std::string>::iterator itr1 = prefixs.begin();
             itr1 != prefixs.end(); itr1++) {
            if (strncmp(itr1->c_str(), endpoint.c_str(), itr1->size()) == 0 &&
                (itr1->size() > holder.size())) {
                holder = *itr1;
                target = *itr;
            }
        }
    }

    // std::cout << "matcher path: " << target.getRoot() << std::endl;
    return target;
}

Server& RequestHandler::validServerName(std::string serverName) {
    for (std::vector<Server>::iterator itr = servers.begin(); itr != servers.end(); itr++) {
        std::vector<std::string> server_names = itr->getServerNames();
        for (std::vector<std::string>::iterator itr1 = server_names.begin(); itr1 != server_names.end(); itr1++) {
            if (serverName == *itr1) {
                return *itr;
            }
        }
    }
    return *(servers.begin());
}

int RequestHandler::getFd() { return this->fd; }

void    RequestHandler::setInitialized(bool handled) {
    this->handled = handled;
}

bool    RequestHandler::hasBeenInitialized() const {
    return handled;
}

std::string
RequestHandler::getFileMimeType(const std::string &file_name) const {
    std::string extention = file_name.substr(file_name.find_last_of('.') + 1);
    return Mime::getInstance()->getMimeType(extention);
}
