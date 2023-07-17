#include "Client.hpp"
#include <cstring>
#include <stdexcept>
#include <unistd.h>

Client::Client(const Server* server) : server(server) {}

int Client::getSocketFd() const {
    return this->socketFd;
}

bool    Client::writeChunk() {
    if (writeBuffer.empty())
        return true;
    int len = write(socketFd, writeBuffer.c_str(), writeBuffer.length());
    if (len == -1)
        throw std::runtime_error(std::string("Client write() error:") + strerror(errno));
    writeBuffer = writeBuffer.substr(len, writeBuffer.length() - len);
    return false;
}

bool    Client::readRequest() {
    server->getHost();
    if (readBuffer.find("\r\n\r\n") != std::string::npos || readBuffer.size() > 8190) {
        // This means the buffer is "full" and we can pass this buffer to the request
        // parser.
    }
    // Else, you keep reading and appending to the buffer (ofc check if read returns end of file then
    // you return true as well. Otherwise you return false)
    return true;
}

// bool    Client::readChunk() {
//     int maxBodySize = server.getClientMaxBodySize(); // Subject to change
//     int maxHeaderSize = 8190;                        // Subject to change
//     // read;
//     // return false;
//     // "\r\n\r\n"
//     // return true;
// }

std::string Client::getRequest() {
    return this->readBuffer;
}

const   Server& Client::getServer() {
    return *this->server;
}

void    Client::storeResponse(const std::string& response) {
    this->writeBuffer = response;
}

Client::~Client() {}
