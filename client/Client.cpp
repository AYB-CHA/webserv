#include "Client.hpp"
#include "../response/HttpResponseException.hpp"
#include <iostream>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

const int Client::read_buf_size = 8190;
const unsigned int Client::max_timeout = 30;

Client::Client() : connectionClose(false), server(NULL) {
    gettimeofday(&lastTimeRW, NULL);
}

Client::Client(const Client& client)
    : socketFd(client.socketFd), writeBuffer(client.writeBuffer),
    connectionClose(client.connectionClose), lastTimeRW(client.lastTimeRW),
    server(client.server) {}

bool    Client::writeChunk() {
    if (writeBuffer.empty() && bodyFd == -1)
        return true;
    if (!writeBuffer.empty()) {
        int len = write(socketFd, writeBuffer.c_str(), writeBuffer.length());
        if (len == -1)
            throw std::runtime_error(std::string("Client write() error:") + strerror(errno));
        writeBuffer = writeBuffer.substr(len, writeBuffer.length() - len);
    } else {
        // if writing from bodyFD returns 0, close it (by throwing
        // a closeConnectionException)
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

unsigned int Client::timeDifference() const {
    timeval current;
    gettimeofday(&current, NULL);

    unsigned int difference = (current.tv_sec - lastTimeRW.tv_sec)
            + (current.tv_usec / 1000000 - lastTimeRW.tv_usec / 1000000);
    return difference;
}

bool    Client::shouldBeClosed() const {
    return this->connectionClose || (timeDifference() > max_timeout);
}

void    Client::setServer(Server *server) {
    this->server = server;
}

void    Client::setFd(int fd) {
    this->socketFd = fd;
}

void    Client::setConnectionClose(bool close) {
    this->connectionClose = close;
}

void    Client::storeResponse(const std::string& response) {
    this->writeBuffer = response;
}

Client::~Client() {}
