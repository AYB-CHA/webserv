#include "Client.hpp"
#include "../request/HttpRequestParser.hpp"
#include "../response/HttpResponseException.hpp"
#include "../server/utils.hpp"
#include "../utils/string.hpp"
#include "FormData.hpp"
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int sendFile(int fileFd, int socketFd, off_t *offset, size_t count);

const unsigned int Client::max_timeout = 30;
const int Client::max_sendfile = 1000000;

Client::Client()
    : bodyFd(-1), cgiReadFd(-1), cgiWriteFd(-1), method("GET"), file_offset(0),
      connectionClose(false), clientMaxBodySize(1024), contentLength(0),
      headersSent(false), chunkedRequest(false), formData(false),
      chunkIsReady(false) {
    gettimeofday(&lastTimeRW, NULL);
}

Client::Client(int socketFd, Server server)
    : socketFd(socketFd), bodyFd(-1), cgiReadFd(-1), cgiWriteFd(-1), method("GET"),
      file_offset(0), connectionClose(false), clientMaxBodySize(1024), contentLength(0),
      headersSent(false), chunkedRequest(false), formData(false),
      chunkIsReady(false), server(server) {
    gettimeofday(&lastTimeRW, NULL);
}

Client::Client(const Client &client)
    : requestHandler(client.requestHandler), socketFd(client.socketFd),
      bodyFd(client.bodyFd), cgiReadFd(client.cgiReadFd), cgiWriteFd(client.cgiWriteFd),
      bufC(client.bufC), method(client.method),
      file_offset(client.file_offset), connectionClose(client.connectionClose),
      clientMaxBodySize(client.clientMaxBodySize),
      contentLength(client.contentLength), lastTimeRW(client.lastTimeRW),
      headersSent(client.headersSent), chunkedRequest(client.chunkedRequest),
      formData(client.formData), formDataBoundary(client.formDataBoundary),
      chunkIsReady(client.chunkIsReady), chunkedLength(client.chunkedLength),
      server(client.server) {}

Client &Client::operator=(const Client &o) {
    if (this == &o)
        return *this;

    this->requestHandler = o.requestHandler;
    this->socketFd = o.socketFd;
    this->bodyFd = o.bodyFd;
    this->cgiReadFd = o.cgiReadFd;
    this->cgiWriteFd = o.cgiWriteFd;
    this->bufC = o.bufC;
    this->method = o.method;
    this->file_offset = o.file_offset;
    this->connectionClose = o.connectionClose;
    this->clientMaxBodySize = o.clientMaxBodySize;
    this->contentLength = o.contentLength;
    this->lastTimeRW = o.lastTimeRW;
    this->headersSent = o.headersSent;
    this->chunkedRequest = o.chunkedRequest;
    this->chunkIsReady = o.chunkIsReady;
    this->chunkedLength = o.chunkedLength;
    this->server = o.server;
    this->formData = o.formData;
    this->formDataBoundary = o.formDataBoundary;
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
        return (readBody());
    }
    return readStatusHeaders();
}

bool Client::readOutputCGI() {
    if (headersSent == false) {
        return readCGIHeaders();
    }
    return readCGIBody();
}

bool Client::writeBodyCGI() {
    int len = write(cgiWriteFd, bufC.body.c_str(), bufC.body.length());
    if (len == -1) {
        throw HttpResponseException(500);
    }
    bufC.body = bufC.body.substr(len);
    return bufC.body.empty();
}

void Client::handleRequest(std::vector<Server> servers, Mediator &mediator) {
    if (!requestHandler.hasBeenInitialized()) {
        HttpRequest request;
        HttpRequestParser parser(request, this->getRequest());
        requestHandler = RequestHandler(request, servers);
        requestHandler.init(*this);
        requestHandler.setInitialized(true);
        if (method == "POST" && (contentLength != 0 || chunkedRequest == false))
            return;
    }

    requestHandler.setInitialized(false);
    if (method == "GET") {
        requestHandler.handleGET(*this, mediator);
    }
    if (method == "POST") {
        this->setMethod("GET");
        requestHandler.handlePOST(*this, mediator);
    }
    // if (method == "DELETE") {
    //     requestHandler.handleDELETE(*this, mediator);
    // }
}

bool Client::writeFromBuffer() {
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

bool Client::writeFromFile() {
    int bytes_sent = sendFile(bodyFd, socketFd, &file_offset, max_sendfile);
    if (bytes_sent > 0)
        updateTimeout();
    if (bytes_sent == 0) {
        clear();
        return true;
    }
    return false;
}

bool Client::readContentLengthBody() {
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
    if (len == -1 || len == 0) {
        connectionClose = true;
        return false;
    }
    updateTimeout();
    return false;
}

bool Client::readFormData() {
    std::vector<char> buf(contentLength);
    int len = read(socketFd, buf.data(), contentLength);
    if (len == -1 || len == 0) {
        connectionClose = true;
        return false;
    }
    this->bufC.formData += std::string(buf.data(), len);
    contentLength -= len;

    std::string::size_type boundary_pos;

    while ((boundary_pos = bufC.formData.find("--" + this->formDataBoundary)) !=
           std::string::npos) {
        std::string to_process = this->bufC.formData.substr(0, boundary_pos);

        if (to_process.length() != 0) {
            if (to_process.find("\r\n") != 0)
                throw HttpResponseException(400);
            const std::string &upload_path =
                (this->requestHandler.matchedLocation())
                    ? this->requestHandler.getLocation().getUploadPath()
                    : this->server.getUploadPath();

            FormData processor(to_process, upload_path);
            processor.processBoundary();
        }
        bufC.formData.erase(0, 2 + this->formDataBoundary.length() +
                                   to_process.length());
    }
    return contentLength == 0;
}

bool Client::readChunkedHexa() {
    char buf[1];

    int len = read(socketFd, buf, 1);
    if (len == -1 || len == 0) {
        connectionClose = true;
        return false;
    }

    bufC.hexa += std::string(buf, len);
    if (bufC.hexa.size() > static_cast<size_t>(clientMaxBodySize)) {
        bufC.hexa.clear();
        throw HttpResponseException(413);
    }
    if (bufC.hexa.find("\r\n") != std::string::npos) {
        if (bufC.hexa.find("\r\n") != bufC.hexa.size() -2) {
            bufC.hexa.clear();
            throw HttpResponseException(400);
        }
        utils::strTrimV2(bufC.hexa, "\r\n");
        chunkedLength = utils::string::toIntHex(bufC.hexa);
        bufC.hexa.clear();
        bufC.temp.resize(bufC.temp.size() + chunkedLength); // wtf?
        chunkIsReady = true;
    }
    return false;
}

bool Client::readChunkedBody() {
    if (!chunkIsReady)
        return readChunkedHexa();
    std::vector<char> buf;
    buf.resize(chunkedLength - bufC.chunk.length() + 2);
    int len =
        read(socketFd, buf.data(), chunkedLength - bufC.chunk.length() + 2);
    if (len == -1 || len == 0) {
        connectionClose = true;
        return false;
    }
    bufC.chunk += std::string(buf.data(), len);
    if (bufC.chunk.length() == chunkedLength + 2) {
        if (bufC.chunk.find("\r\n") == std::string::npos || bufC.chunk.find("\r\n") != bufC.chunk.size() - 2)
            throw HttpResponseException(400);
        chunkIsReady = false;
        if (chunkedLength == 0) {
            bufC.chunk.clear();
            chunkedRequest = false;
            return true;
        }
        utils::strTrimV2(bufC.chunk, "\r\n");
        bufC.body += bufC.chunk;
        if (bufC.body.size() > static_cast<size_t>(clientMaxBodySize)) {
            bufC.body.clear();
            throw HttpResponseException(413);
        }
        bufC.chunk.clear();
    }
    return false;
}

bool Client::readBody() {
    // std::cout << formData << std::endl;
    if (chunkedRequest == true)
        return readChunkedBody();
    else if (formData == true) {
        if (this->readFormData() == true) {
            HttpResponseBuilder response;
            response.setStatuscode(201);
            this->bufC.write = response.build();
        }
        return false;
    }
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
    int len = read(cgiReadFd, buf, 1);
    if (len <= 0) {
        throw HttpResponseException(500);
    }
    bufC.headers += std::string(buf, 1);
    if (bufC.headers.find("\r\n\r\n") != std::string::npos) {
        bufC.write = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n" + bufC.headers;
        bufC.headers.clear();
        headersSent = true;
    }
    return false;
}

bool Client::readCGIBody() {
    char buf[1024];
    int len = read(cgiReadFd, buf, 1024);
    if (len == -1) {
        throw HttpResponseException(500);
    }
    std::string readString = std::string(buf, len);
    std::string append =
        utils::string::toHex(len) + "\r\n" + readString + "\r\n";
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

int Client::getCgiReadFd() const { return this->cgiReadFd; }

int Client::getCgiWriteFd() const { return this->cgiWriteFd; }

std::string Client::getRequest() {
    std::string ret = bufC.read;
    bufC.read.clear();

    return ret;
}

std::string Client::getMethod() const { return this->method; }

Server &Client::getServer() { return this->server; }

const std::string &Client::getPostBody() { return this->bufC.body; }

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
    return (this->connectionClose && bufC.write.empty() && bodyFd == -1 &&
            cgiReadFd == -1 /* && cgiWriteFd == -1 */) ||
           (timeDifference() > max_timeout);
}

void Client::setServer(Server server) { this->server = server; }

void Client::setFd(int fd) { this->socketFd = fd; }

void Client::setFileFd(int fd) { this->bodyFd = fd; }

void Client::setCgiReadFd(int fd) { this->cgiReadFd = fd; }

void Client::setCgiWriteFd(int fd) { this->cgiWriteFd = fd; }

void Client::setMethod(const std::string &method) { this->method = method; }

void Client::setContentLength(off_t length) { this->contentLength = length; }

void Client::setConnectionClose(bool close) { this->connectionClose = close; }

void Client::setChunkedRequest(bool chunked) { this->chunkedRequest = chunked; }
void Client::setFormData(bool val) { this->formData = val; }
void Client::setFormDataBoundary(const std::string &boundary) {
    this->formDataBoundary = boundary;
}

void Client::storeResponse(const std::string &response) {
    clear();
    this->bufC.write = response;
}

void Client::reset() {
    if (bufC.write.empty() && bodyFd == -1 && cgiReadFd == -1)
        *this = Client(socketFd, server);
}

void Client::clear() {
    bufC.write.clear();
    if (bodyFd != -1)
        close(bodyFd);
    bodyFd = -1;
    file_offset = 0;
}

RequestHandler &Client::getRequestHandler() { return this->requestHandler; }

void Client::showErrorPage(HttpResponseException &e) {
    const std::map<int, std::string> &error_pages =
        (this->getRequestHandler().matchedLocation())
            ? this->getRequestHandler().getLocation().getErrorPage()
            : this->getServer().getErrorPage();

    if (error_pages.find(e.getStatusCode()) != error_pages.end()) {
        std::string file_path = error_pages.at(e.getStatusCode());
        int fd = open(file_path.c_str(), O_RDONLY);
        if (fd != -1) {
            HttpResponseBuilder builder;
            std::string extention =
                file_path.substr(file_path.find_last_of('.') + 1);
            struct stat data;
            fstat(fd, &data);
            builder.setStatuscode(200)
                ->setHeader("Content-Type",
                            Mime::getInstance()->getMimeType(extention))
                ->setHeader("Content-Length",
                            ::utils::string::fromInt(data.st_size));

            this->storeResponse(builder.build());
            this->setFileFd(fd);
            return;
        }
    }
    this->storeResponse(e.build());
}

bool Client::getChunkedRequestStatus() const { return this->chunkedRequest; }
bool Client::getFormDataStatus() const { return this->formData; }

Client::~Client() {}
