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
#include "../server/utils.hpp"
#include <unistd.h>
int sendFile(int fileFd, int socketFd, off_t *offset, size_t count);

const unsigned int Client::max_timeout = 30;
const int Client::max_sendfile = 1000000;

Client::Client()
    : bodyFd(-1), cgiFd(-1), method("GET"), file_offset(0),
      connectionClose(false), clientMaxBodySize(1024),
      contentLength(0), headersSent(false), chunkedRequest(false)
{
    gettimeofday(&lastTimeRW, NULL);
}

Client::Client(int socketFd, Server server) 
    : socketFd(socketFd), bodyFd(-1), cgiFd(-1), method("GET"), file_offset(0),
      connectionClose(false), clientMaxBodySize(1024),
      contentLength(0), headersSent(false), chunkedRequest(false),
      server(server)
{
    gettimeofday(&lastTimeRW, NULL);
}

Client::Client(const Client &client)
    : requestHandler(client.requestHandler),
      socketFd(client.socketFd), bodyFd(client.bodyFd), cgiFd(client.cgiFd),
      bufC(client.bufC), method(client.method),
      file_offset(client.file_offset), connectionClose(client.connectionClose),
      clientMaxBodySize(client.clientMaxBodySize), contentLength(client.contentLength),
      lastTimeRW(client.lastTimeRW), headersSent(client.headersSent), 
      chunkedRequest(client.chunkedRequest), server(client.server) {}

Client& Client::operator=(const Client& o) {
    if (this == &o) return *this;
    new (this) Client(o);
    return *this;
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
    }
    return readStatusHeaders();
}

bool Client::readOutputCGI() {
    if (headersSent == false) {
        return readCGIHeaders();
    }
    return readCGIBody();
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
    // if (method == "DELETE") {
    //     requestHandler.handleDELETE(*this, mediator);
    // }
}

bool    Client::writeFromBuffer() {
    const char *string = bufC.write.c_str();
    size_t length = bufC.write.length();

    int writeLen = write(socketFd, string, length);
    if (writeLen == -1) {
        clear();
        connectionClose = true;
        return false;
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
        clear();
        return true;
    }
    return false;
}

bool    Client::readContentLengthBody() {
    const size_t previousSize = bufC.temp.size();

    bufC.temp.resize(bufC.temp.size() + contentLength);
    int len = read(socketFd, bufC.temp.data() + previousSize, contentLength);
    bufC.temp.resize(previousSize + len);

    contentLength -= len;
    if (contentLength == 0) {
        bufC.body = std::string(bufC.temp.begin(), bufC.temp.end());
        bufC.temp.clear();
        updateTimeout();
        return true;
    }
    if (len == 0) {
        throw HttpResponseException(400);
    }
    if (len == -1) {
        connectionClose = true;
        return false;
    }
    updateTimeout();
    return false;
}

bool    Client::readChunkedHexa() {
    char buf[1];

    int len = read(socketFd, buf, 1);
    if (len == -1 || len == 0) {
        connectionClose = true;
        return false;
    }

    bufC.hexa += std::string(buf, len);
    if (bufC.hexa.find("\r\n") != std::string::npos) {
        utils::strTrimV2(bufC.hexa, "\r\n");
        chunkedLength = utils::string::toIntHex(bufC.hexa) + 2;
        bufC.hexa.clear();
        bufC.temp.resize(bufC.temp.size() + chunkedLength);
        chunkIsReady = true;
    }
    return false;
}

bool    Client::readChunkedBody() {
    if (!chunkIsReady)
        return readChunkedHexa();
    char buf[1024];
    int len = read(socketFd, buf, chunkedLength - bufC.chunk.length() + 2);
    if (len == -1 || len == 0) {
        connectionClose = true;
        return false;
    }
    bufC.chunk += std::string(buf, len);
    if (bufC.chunk.length() == chunkedLength + 2) {
        if (bufC.chunk.find("\r\n") == std::string::npos)
            throw HttpResponseException(400);
        chunkIsReady = false;
        if (chunkedLength == 0) {
            bufC.chunk.clear();
            chunkedRequest = false;
            return true;
        }
        utils::strTrimV2(bufC.chunk, "\r\n");
        bufC.body += bufC.chunk;
        bufC.chunk.clear();
    }
    return false;
}

bool    Client::readBody() {
    if (chunkedRequest == true)
        return readChunkedBody();
    return readContentLengthBody();
}

bool Client::readStatusHeaders() {
    char buffer[1];
    int readlen = recv(socketFd, buffer, 1, 0);

    if (readlen == -1 || readlen == 0) {
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

bool Client::readCGIHeaders() {
    char buf[1];
    int len = read(cgiFd, buf, 1);
    if (len <= 0) {
        throw HttpResponseException(500);
    }
    bufC.headers += std::string(buf, 1);
    if (bufC.headers.find("\r\n\r\n") != std::string::npos) {
        bufC.write += bufC.headers;
        headersSent = true;
    }
    return false;
}

bool Client::readCGIBody() {
    char buf[1024];
    int len = read(cgiFd, buf, 1024);
    if (len == -1) {
        throw HttpResponseException(500);
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

void Client::setChunkedRequest(bool chunked) { this->chunkedRequest = chunked; }

void Client::storeResponse(const std::string &response) {
    clear();
    this->bufC.write = response;
}

void Client::reset() {
    if (bufC.write.empty() && bodyFd == -1 && cgiFd == -1)
        *this = Client(socketFd, server);
}

void Client::clear() {
    bufC.write.clear();
    if (bodyFd != -1)
        close(bodyFd);
    bodyFd = -1;
    if (cgiFd != -1)
        close(cgiFd);
    cgiFd = -1;
    file_offset = 0;
}

Client::~Client() {}
