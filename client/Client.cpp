#include "Client.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

const int Client::read_buf_size = 8190;

Client::Client(const Server* server) : requestRead(false), server(server) {}

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
    size_t it = readBuffer.find("\r\n\r\n");
    if (it != std::string::npos || readBuffer.size() > 8190) { //separate the two conditions, cuz it could be npos
        bodyBuffer = readBuffer.substr(it, readBuffer.size() - it);
        readBuffer = readBuffer.substr(0, readBuffer.size() - bodyBuffer.size());
        return true;
    }
    char buffer[Client::read_buf_size];
    int readlen = recv(this->socketFd, buffer, Client::read_buf_size, 0);
    if (readlen == 0)
        throw closeConnectionException();
    readBuffer += std::string(buffer, readlen);
    return false;
}

// bool    Client::readChunk() {
//     int maxBodySize = server.getClientMaxBodySize(); // Subject to change
//     int maxHeaderSize = 8190;                        // Subject to change
//     // read;
//     // return false;
//     // "\r\n\r\n"
//     // return true;
// }

int Client::getSocketFd() const {
    return this->socketFd;
}

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
