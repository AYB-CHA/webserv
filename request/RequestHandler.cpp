#include "RequestHandler.hpp"
#include "../response/HttpResponseException.hpp"

#include <unistd.h> // access
#include <fstream>

RequestHandler::RequestHandler(HttpRequest &request, Server &server) {
    this->request = request;
    this->server = server;
    handleIt();
}

std::string RequestHandler::getResponse() {
    return response.build();
}

void RequestHandler::handleIt() {

    std::string file = request.getEndpoint();
    file = "." + file;
    std::cout << file << std::endl;
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
