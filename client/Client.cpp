#include "Client.hpp"
#include "../response/HttpResponseException.hpp"
#include <iostream>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

const int Client::read_buf_size = 8190;

Client::Client() : connectionClose(false), server(NULL) {}

Client::Client(const Client& client) : socketFd(client.socketFd), writeBuffer(client.writeBuffer), connectionClose(client.connectionClose), server(client.server) {}

bool    Client::writeChunk() {
    if (writeBuffer.empty() && bodyBuffer.empty()) // Client.isReadingBody()) 
        return true;
    if (!writeBuffer.empty()) {
        int len = write(socketFd, writeBuffer.c_str(), writeBuffer.length());
        if (len == -1)
            throw std::runtime_error(std::string("Client write() error:") + strerror(errno));
        writeBuffer = writeBuffer.substr(len, writeBuffer.length() - len);
    } else {
        // if writing from bodyFD returns 0, close it
        //sendfile(2)
    }
    return false;
}

bool    Client::readRequest() {
    size_t it = readBuffer.find("\r\n\r\n");
    if (readBuffer.size() >= Client::read_buf_size) {
        connectionClose = true;
        throw HttpResponseException(494);
    }
    if (it != std::string::npos) {
        bodyBuffer = readBuffer.substr(it, readBuffer.size() - it);
        readBuffer = readBuffer.substr(0, readBuffer.size() - bodyBuffer.size());
        return true;
    }
    char buffer[Client::read_buf_size];
    int readlen = recv(this->socketFd, buffer, Client::read_buf_size, 0);
    if (readlen == -1)
        throw std::runtime_error(std::string("readlen(): ") + strerror(errno));
    if (readlen == 0)
        throw closeConnectionException();
    readBuffer += std::string(buffer, readlen);
    return true;
}

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

bool    Client::shouldBeClosed() const {
    // In this function, you also check for timeout.
    // So if the connection times out or the Connection: close
    // header is set, then we close it
    return this->connectionClose;
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
