#include "RequestHandler.hpp"
#include <sys/fcntl.h>
#include <sys/stat.h>
#include "../response/HttpResponseException.hpp"
#include "../utils/string.hpp"

#include <unistd.h> // access
#include <cstring>
#include <fstream>

#include <vector>

RequestHandler::RequestHandler(HttpRequest &request, Client &client) {
    this->request = request;
    this->client = client;
    fd = -1;
    handleIt();
}

std::string RequestHandler::getResponse() {
    return response.build();
}

Location RequestHandler::matchLocation(std::string endpoint, std::vector<Location>& locations) {
    // std::vector<Location> locations;
    Location target;
    std::string holder = "";

    for (std::vector<Location>::iterator itr = locations.begin(); itr != locations.end(); itr++) {
        std::vector<std::string> prefixs = itr->getPrefix();
        for (std::vector<std::string>::iterator itr1 = prefixs.begin(); itr1 != prefixs.end(); itr1++) {
            if (strncmp(itr1->c_str(), endpoint.c_str(), itr1->size()) == 0 && (itr1->size() > holder.size())) {
                holder = *itr1;
                target = *itr;
            }
        }
    }

    // std::cout << "matcher path: " << target.getRoot() << std::endl;
    return target;
}

void validPath() {

}

void RequestHandler::handleIt() {

    std::string file = request.getEndpoint();

    Location targetLoc = matchLocation(file, const_cast<std::vector<Location>& >(client.getServer().getLocation()));

    file = "./" + targetLoc.getRoot() + file;

    // std::cout << file << std::endl;
    if (access(file.c_str(), F_OK) == -1)
        throw HttpResponseException(404);
    if (access(file.c_str(), R_OK == -1))
        throw HttpResponseException(403);

    struct stat data;
    stat(file.c_str(), &data);
    off_t length = data.st_size;

    fd = open(file.c_str(), O_RDONLY);

    std::cout << "aaasilen: " << length << std::endl;
    // std::cout << "fd: " << fd << std::endl;
    response.setStatuscode(200)
        // ->setHeader("Content-Type", "video/mp4")
        ->setHeader("Content-Length", utils::string::fromInt(length));
}

int    RequestHandler::getFd() {
    return this->fd;
}
