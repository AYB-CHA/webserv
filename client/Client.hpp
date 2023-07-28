#pragma once

#include "../server/Server.hpp"
#include <sys/time.h>
#include <cerrno>
#include <exception>
#include <string>
#include <sys/types.h>
#include "../request/RequestHandler.hpp"

class Client {
private:
    static const unsigned int max_timeout;
    static const int max_sendfile;
    struct BufferContainer {
        std::string write;
        std::string body;
        std::string read;
        std::vector<char> temp;
    };

    RequestHandler requestHandler;

    int     socketFd;
    int     bodyFd;

    BufferContainer bufC;
    std::string method;

    off_t   file_offset;
    bool    connectionClose;
    off_t   clientMaxBodySize;
    off_t   contentLength;
    timeval lastTimeRW;
    bool    hasReadPostBody;
    bool    cgiIsSet;

    Server  server;

    unsigned int timeDifference() const;
    bool    writeFromBuffer();
    bool    writeFromFile();
    bool    readBody();
    bool    readStatusHeaders();
    void    updateTimeout();
    void    readOutputCGI();
public:
    Client();
    Client(const Client& o);
    bool    operator==(const Client& o) const;

    int     getSocketFd() const;
    std::string getMethod() const;
    std::string getRequest();
    Server& getServer();
    std::string getPostBody();
    bool    shouldBeClosed() const;
    bool    hasReadBody() const;

    void    setServer(Server server);
    void    setFd(int fd);
    void    setFileFd(int fd);
    void    setMethod(const std::string& method);
    void    setContentLength(off_t length);
    void    setConnectionClose(bool close);

    bool    writeChunk();
    bool    readRequest();
    void    handleRequest(std::vector<Server> servers);
    void    storeResponse(const std::string& response);

    ~Client();
};
