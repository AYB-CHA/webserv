#include "Client.hpp"
#include "../response/HttpResponseException.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
int sendFile(int fileFd, int socketFd, off_t *offset, size_t count);

const int Client::read_buf_size = 1;
const unsigned int Client::max_timeout = 30;
const int Client::max_sendfile = 1000000;

Client::Client()
    : bodyFd(-1), method("GET"), file_offset(0),
      connectionClose(false), clientMaxBodySize(1024) {
    gettimeofday(&lastTimeRW, NULL);
}

Client::Client(const Client &client)
    : socketFd(client.socketFd), bodyFd(client.bodyFd),
      writeBuffer(client.writeBuffer), bodyBuffer(client.bodyBuffer),
      readBuffer(client.readBuffer), method(client.method), tempBuffer(client.tempBuffer),
      file_offset(client.file_offset), connectionClose(client.connectionClose),
      clientMaxBodySize(client.clientMaxBodySize), contentLength(client.contentLength),
      lastTimeRW(client.lastTimeRW), server(client.server) {}

bool Client::writeChunk() {
    if (writeBuffer.empty() && bodyFd == -1)
        return true;
    if (!writeBuffer.empty()) {
        int len = write(socketFd, writeBuffer.c_str(), writeBuffer.length());
        if (len == -1)
            throw std::runtime_error(std::string("Client write() error:") +
                                     strerror(errno));
        writeBuffer = writeBuffer.substr(len, writeBuffer.length() - len);
        updateTimeout();
    } else {
        int bytes_sent = sendFile(bodyFd, socketFd, &file_offset, max_sendfile);
        // std::cout << "bytes sent: " << bytes_sent << std::endl;
        if (bytes_sent > 0)
            updateTimeout();
        if (bytes_sent == 0) {
            close(bodyFd);
            bodyFd = -1;
            file_offset = 0;
            return true;
        }
    }
    return false;
}

bool    Client::readBody() {
    const size_t previousSize = tempBuffer.size();

    tempBuffer.resize(tempBuffer.size() + contentLength);
    int len = read(socketFd, tempBuffer.data() + tempBuffer.size(), contentLength);
    tempBuffer.resize(previousSize + len);

    contentLength -= len;
    if (contentLength == 0) {
        bodyBuffer = std::string(tempBuffer.begin(), tempBuffer.end());
        method = "GET";
        return true;
    }
    if (len <= 0) {
        connectionClose = true;
        return false;
    }
    return false;
}

bool Client::readRequest() {
    if (method == "POST") {
        return readBody();
    }
    char buffer[Client::read_buf_size];
    int readlen = recv(this->socketFd, buffer, Client::read_buf_size, 0);

    if (readlen == -1)
        throw std::runtime_error(std::string("readlen(): ") + strerror(errno));
    if (readlen == 0) {
        connectionClose = true;
        return false;
    }
    readBuffer += std::string(buffer, readlen);
    updateTimeout();

    if (readBuffer.size() >= 8190) {
        connectionClose = true;
        throw HttpResponseException(494);
    }
    if (readBuffer.find("\r\n\r\n") != std::string::npos) {
        return true;
    }
    return false;
}

bool Client::operator==(const Client &o) const {
    return this->socketFd == o.socketFd;
}

int Client::getSocketFd() const { return this->socketFd; }

std::string Client::getRequest() {
    std::string ret = this->readBuffer;
    this->readBuffer.clear();

    return ret;
}

Server &Client::getServer() { return this->server; }

std::string Client::getPostBody() {
    return this->bodyBuffer;
}

unsigned int Client::timeDifference() const {
    timeval current;
    gettimeofday(&current, NULL);

    return (current.tv_sec - lastTimeRW.tv_sec) +
           (current.tv_usec / 1000000 - lastTimeRW.tv_usec / 1000000);
}

void Client::updateTimeout() {
    if (gettimeofday(&lastTimeRW, NULL) == -1)
        throw std::runtime_error(std::string("gettimeofday(): ") +
                                 strerror(errno));
}

bool Client::shouldBeClosed() const {
    return (this->connectionClose && writeBuffer.empty() && bodyFd == -1) ||
           (timeDifference() > max_timeout);
}

void Client::setServer(Server server) { this->server = server; }

void Client::setFd(int fd) { this->socketFd = fd; }

void Client::setFileFd(int fd) { this->bodyFd = fd; }

void Client::setConnectionClose(bool close) { this->connectionClose = close; }

void Client::storeResponse(const std::string &response) {
    this->writeBuffer = response;
}

Client::~Client() {}
