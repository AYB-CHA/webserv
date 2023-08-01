#include "Client.hpp"
#include "../response/HttpResponseException.hpp"
#include "../request/HttpRequestParser.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include "../utils/string.hpp"
#include <unistd.h>
int sendFile(int fileFd, int socketFd, off_t *offset, size_t count);

const unsigned int Client::max_timeout = 30;
const int Client::max_sendfile = 1000000;

Client::Client()
    : bodyFd(-1), cgiFd(-1), method("GET"), file_offset(0),
      connectionClose(false), clientMaxBodySize(1024),
      contentLength(0),
      headersSent(false)
{
    gettimeofday(&lastTimeRW, NULL);
}

Client::Client(int socketFd, Server server) 
    : bodyFd(-1), cgiFd(-1), method("GET"), file_offset(0),
      connectionClose(false), clientMaxBodySize(1024),
      contentLength(0),
      headersSent(false)
{
    this->socketFd = socketFd;
    this->server = server;
}

Client::Client(const Client &client)
    : requestHandler(client.requestHandler),
      socketFd(client.socketFd), bodyFd(client.bodyFd), cgiFd(client.cgiFd),
      bufC(client.bufC), method(client.method),
      file_offset(client.file_offset), connectionClose(client.connectionClose),
      clientMaxBodySize(client.clientMaxBodySize), contentLength(client.contentLength),
      lastTimeRW(client.lastTimeRW),
      headersSent(client.headersSent), server(client.server) {}

Client& Client::operator=(const Client& o) {
    if (this == &o) return *this;
    new (this) Client(o);
    return *this;
}

bool Client::readOutputCGI() {
    if (headersSent == false) {
        char buf[1];
        int len = read(cgiFd, buf, 1);
        if (len <= 0) {
            // make sure you reset things to not interefere with the respone writing
            // add a method called reset() that handles that
            throw HttpResponseException(400);
        }
        bufC.headers += std::string(buf, 1);
        if (bufC.headers.find("\r\n\r\n") != std::string::npos) {
            bufC.write += bufC.headers;
            headersSent = true;
        }
        return false;
    }
    char buf[1024];
    int len = read(cgiFd, buf, 1024);
    if (len == -1) {
        std::string errMsg("readOutputCGI(): ");
        throw std::runtime_error(errMsg + strerror(errno));
    }
    std::string readString = std::string(buf, len);
    std::string append = utils::string::toHex(len) + "\r\n" + readString + "\r\n";
    bufC.write += append;
    if (len == 0) {
        headersSent = false;
        bufC.headers.clear();
        return true;
    }
    return false;
}

bool Client::writeChunk() {
    if (bufC.write.empty() && bodyFd == -1)
        return true;
    if (!bufC.write.empty()) {
        return writeFromBuffer();
    }
    return writeFromFile();
}

bool Client::readRequest() {
    if (method == "POST") {
        return readBody();
    } else if (method == "DELETE") {
        return true; // Unimplemented
    } else {
        return readStatusHeaders();
    }
}

void Client::handleRequest(std::vector<Server> servers, Mediator& mediator) {
    if (!requestHandler.hasBeenInitialized()) {
        HttpRequest request;
        HttpRequestParser parser(request, this->getRequest());
        requestHandler = RequestHandler(request, servers);
        requestHandler.init(*this);
        requestHandler.setInitialized(true);
        if (method == "POST")
            return;
    }
    requestHandler.setInitialized(false);
    if (method == "GET") {
        requestHandler.handleGET(*this, mediator);
    }
    if (method == "POST") {
        requestHandler.handlePOST(*this, mediator);
    }
}

bool    Client::writeFromBuffer() {
    const char *string = bufC.write.c_str();
    size_t length = bufC.write.length();

    int writeLen = write(socketFd, string, length);
    if (writeLen == -1) {
        clear();
        connectionClose = true;
        return false;
        // std::string errMsg("Client write() error:");
        // throw std::runtime_error(errMsg + strerror(errno));
    }

    size_t remainingLength = length - writeLen;
    bufC.write = bufC.write.substr(writeLen, remainingLength);
    updateTimeout();
    return false;
}

bool    Client::writeFromFile() {
    int bytes_sent = sendFile(bodyFd, socketFd, &file_offset, max_sendfile);
    if (bytes_sent > 0)
        updateTimeout();
    if (bytes_sent == 0) {
        std::cout << "I have closed the file." << std::endl;
        close(bodyFd);
        bodyFd = -1;
        file_offset = 0;
        return true;
    }
    return false;
}

bool    Client::readBody() {
    const size_t previousSize = bufC.temp.size();

    bufC.temp.resize(bufC.temp.size() + contentLength);
    int len = read(socketFd, bufC.temp.data() + previousSize, contentLength);
    bufC.temp.resize(previousSize + len);

    contentLength -= len;
    if (contentLength == 0) {
        bufC.body = std::string(bufC.temp.begin(), bufC.temp.end());
        bufC.temp.clear();
        return true;
    }
    if (len <= 0) {
        connectionClose = true;
        return false;
    }
    return false;
}

bool Client::readStatusHeaders() {
    char buffer[1];
    int readlen = recv(socketFd, buffer, 1, 0);

    if (readlen == -1) {
        return true;
    }
    if (readlen == 0) {
        connectionClose = true;
        return false;
    }
    bufC.read += std::string(buffer, readlen);
    updateTimeout();

    if (bufC.read.find("\r\n\r\n") != std::string::npos) {
        return true;
    }
    if (bufC.read.size() >= 8190) {
        connectionClose = true;
        throw HttpResponseException(494);
    }
    return false;
}

bool Client::operator==(const Client &o) const {
    return this->socketFd == o.socketFd;
}

int Client::getSocketFd() const { return this->socketFd; }

int Client::getCgiFd() const { return this->cgiFd; }

std::string Client::getRequest() {
    std::string ret = bufC.read;
    bufC.read.clear();

    return ret;
}

std::string Client::getMethod() const {
    return this->method;
}

Server &Client::getServer() { return this->server; }

std::string Client::getPostBody() {
    return this->bufC.body;
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
    return (this->connectionClose && bufC.write.empty() && bodyFd == -1) ||
           (timeDifference() > max_timeout);
}

void Client::setServer(Server server) { this->server = server; }

void Client::setFd(int fd) { this->socketFd = fd; }

void Client::setFileFd(int fd) { this->bodyFd = fd; }

void Client::setCgiFd(int fd) { this->cgiFd = fd; }

void Client::setMethod(const std::string& method) { this->method = method; }

void Client::setContentLength(off_t length) { this->contentLength = length; }

void Client::setConnectionClose(bool close) { this->connectionClose = close; }

void Client::storeResponse(const std::string &response) {
    this->bufC.write = response;
}

void Client::reset() {
    if (bufC.write.empty() && bodyFd == -1 && cgiFd == -1)
        *this = Client(socketFd, server);
}

void Client::clear() {
    bufC.write.clear();
    bodyFd = -1;
    cgiFd = -1;
}

Client::~Client() {}
