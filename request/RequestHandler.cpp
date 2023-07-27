#include "RequestHandler.hpp"
#include "../response/HttpResponseException.hpp"
#include "../response/Mime.hpp"
#include "../utils/string.hpp"
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>
#include <unistd.h> // access

#include <vector>

RequestHandler::RequestHandler(HttpRequest &request, Client &client, std::vector<Server> servers) {
    this->request = request;
    this->client = client;
    fd = -1;
    this->servers = servers;
    handleIt();
}

std::string RequestHandler::getResponse() { return response.build(); }

bool RequestHandler::matchLocation(std::string endpoint, const Server &serv, Location &target) {

    std::vector<Location> locations = serv.getLocation();
    if (locations.empty()) {
        return false;
    }

    bool found = false;
    std::string holder = "";

    forEach (std::vector<Location>, locations, itr) {
        forEachConst (std::vector<std::string>, itr->getPrefix(), itr1) {
            if (strncmp(itr1->c_str(), endpoint.c_str(), itr1->size()) == 0
                && (itr1->size() > holder.size())) {
                found = true;
                holder = *itr1;
                target = *itr;
            }
        }
    }

    return found;
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

void RequestHandler::handleIt() {

    Server srv;

    std::string hostHeader = request.getHeader("Host");
    srv = validServerName(hostHeader);
    client.setServer(srv);
    std::cout << "updated server: " << srv.getServerNames()[0] << std::endl;



    std::string file = request.getEndpoint();
    std::cout << "end point: " << file << std::endl;

    Location targetLoc;
    bool funStatus = matchLocation(file, client.getServer(), targetLoc);
    if (funStatus) {
        std::string LocationRoot = targetLoc.getRoot();
        if (LocationRoot.empty())
            LocationRoot = client.getServer().getRoot();
        file = "." + LocationRoot + file;
    } else {
        file = "." + client.getServer().getRoot() + file;
    }

    // file = "./types.txt";
    std::cout << "-->file: " << file << std::endl;
    std::cout << "-->function state: " << std::boolalpha << funStatus << std::endl;
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
}

int RequestHandler::getFd() { return this->fd; }

std::string
RequestHandler::getFileMimeType(const std::string &file_name) const {
    std::string extention = file_name.substr(file_name.find_last_of('.') + 1);
    return Mime::getInstance()->getMimeType(extention);
}
