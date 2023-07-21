#include "RequestHandler.hpp"
#include "../response/HttpResponseException.hpp"

#include <unistd.h> // access
#include <fstream>

#include <vector>

RequestHandler::RequestHandler(HttpRequest &request, Server &server) {
    this->request = request;
    this->server = server;
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
    file = "." + file;

    Location targetLoc = matchLocation(file, const_cast<std::vector<Location>& >(server.getLocation()));



    // std::cout << file << std::endl;
    if (access(file.c_str(), F_OK) == -1)
        throw HttpResponseException(404);
    if (access(file.c_str(), R_OK == -1))
        throw HttpResponseException(403);

    std::ifstream srcFile (file.c_str(), std::ifstream::binary);
    char* buffer = NULL;
    std::string str;
    if (srcFile) {
        // get length of file:
        srcFile.seekg (0, srcFile.end);
        int length = srcFile.tellg();
        srcFile.seekg (0, srcFile.beg);

        buffer = new char [length];

        // read data as a block:
        srcFile.read (buffer,length);
        if (!srcFile) {
            delete [] buffer;
            throw HttpResponseException(500);
        }

        srcFile.close();
        str += buffer;
    }

    response.pushBody(str)->setStatuscode(200);
    delete[] buffer;
}
