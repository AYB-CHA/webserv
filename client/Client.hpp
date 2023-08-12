#pragma once

#include "../request/RequestHandler.hpp"
#include "../response/HttpResponseException.hpp"
#include "../response/Mime.hpp"
#include "../server/Server.hpp"
#include <cerrno>
#include <exception>
#include <string>
#include <sys/time.h>
#include <sys/types.h>

class Mediator;

class Client {
  private:
    static const unsigned int max_timeout;
    static const int max_sendfile;
    struct BufferContainer {
        std::string write;
        std::string body;
        std::string read;
        std::string headers;
        std::string hexa;
        std::string chunk;
        std::string formData;
        std::vector<char> temp;
    };

    RequestHandler requestHandler;

    int socketFd;
    int bodyFd;
    int cgiReadFd;
    int cgiWriteFd;

    BufferContainer bufC;
    std::string method;

    off_t file_offset;
    bool connectionClose;
    off_t clientMaxBodySize;
    off_t contentLength;
    timeval lastTimeRW;
    bool headersSent;
    bool chunkedRequest;
    bool formData;
    std::string formDataBoundary;
    bool chunkIsReady;
    std::string::size_type chunkedLength;

    Server server;

    unsigned int timeDifference() const;
    bool writeFromBuffer();
    bool writeFromFile();
    bool readBody();
    bool readStatusHeaders();
    bool readCGIHeaders();
    bool readCGIBody();
    bool readContentLengthBody();
    bool readFormData();
    bool readChunkedBody();
    bool readChunkedHexa();
    void updateTimeout();

  public:
    Client();
    Client(int socketFd, Server server);
    Client(const Client &o);
    Client &operator=(const Client &o);
    bool operator==(const Client &o) const;

    int getSocketFd() const;
    int getCgiReadFd() const;
    int getCgiWriteFd() const;
    std::string getMethod() const;
    std::string getRequest();
    Server &getServer();
    RequestHandler &getRequestHandler();
    const std::string &getPostBody();
    bool shouldBeClosed() const;

    void setServer(Server server);
    void setFd(int fd);
    void setFileFd(int fd);
    void setCgiReadFd(int fd);
    void setCgiWriteFd(int fd);
    void setMethod(const std::string &method);
    void setContentLength(off_t length);
    void setConnectionClose(bool close);
    void setChunkedRequest(bool chunked);
    void setFormData(bool val);
    void setFormDataBoundary(const std::string &boundary);

    bool readOutputCGI();
    bool writeChunk();
    bool readRequest();
    void handleRequest(std::vector<Server> servers, Mediator &mediator);
    void storeResponse(const std::string &response);
    void showErrorPage(HttpResponseException &e);
    void reset();
    void clear();
    bool getChunkedRequestStatus() const;
    bool getFormDataStatus() const;
    ~Client();
};
