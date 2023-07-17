#include "Client.hpp"
#include <cstring>
#include <stdexcept>
#include <unistd.h>

Client::Client(const Server& server) : server(server) {}

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

Client::~Client() {}
