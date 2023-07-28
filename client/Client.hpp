#pragma once

#include "../server/Server.hpp"
#include <sys/time.h>
#include <cerrno>
#include <exception>
#include <string>
#include <sys/types.h>

struct BufferContainer {
    std::string write;
    std::string body;
    std::string read;
    std::vector<char> temp;
};

class Client {
private:
    static const unsigned int max_timeout;
    static const int max_sendfile;

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

    Server  server;

    unsigned int timeDifference() const;
    bool    writeFromBuffer();
    bool    writeFromFile();
    bool    readBody();
    bool    readStatusHeaders();
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
    // After the client holds the request handler, storeResponse should be private
    void    storeResponse(const std::string& response);
    void    updateTimeout();

    ~Client();
};
