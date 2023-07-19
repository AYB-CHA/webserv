#include <iostream>
#include "Client.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

const int Client::read_buf_size = 8190;

Client::Client() : requestRead(false), server(NULL) {}

Client::Client(const Client& client) : requestRead(client.requestRead), socketFd(client.socketFd), server(client.server) {}

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
    size_t it = readBuffer.find("\r\n\r\n");
    if (it != std::string::npos || readBuffer.size() > 8190) { //separate the two conditions, cuz it could be npos
        bodyBuffer = readBuffer.substr(it, readBuffer.size() - it);
        readBuffer = readBuffer.substr(0, readBuffer.size() - bodyBuffer.size());
        return true;
    }
    char buffer[Client::read_buf_size];
    std::cout << "client fd: " << this->socketFd << std::endl;
    int readlen = recv(this->socketFd, buffer, Client::read_buf_size, 0);
    if (readlen == -1)
        throw std::runtime_error(std::string("readlen(): ") + strerror(errno));
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

bool    Client::operator==(const Client& o) const {
    return this->socketFd == o.socketFd;
}

int Client::getSocketFd() const {
    return this->socketFd;
}

std::string Client::getRequest() {
    return this->readBuffer;
}

Server& Client::getServer() {
    return *this->server;
}

void    Client::setServer(Server *server) {
    this->server = server;
}

void    Client::setFd(int fd) {
    this->socketFd = fd;
}

void    Client::storeResponse(const std::string& response) {
    this->writeBuffer = response;
}

Client::~Client() {}
