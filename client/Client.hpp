#pragma once

#include "../server/Server.hpp"
#include <sys/time.h>
#include <cerrno>
#include <exception>
#include <string>
#include <sys/types.h>

class Client {
private:
    static const int read_buf_size;
    static const unsigned int max_timeout;
    static const int max_sendfile;

    int     socketFd;
    int     bodyFd;

    std::string writeBuffer;
    std::string bodyBuffer;
    std::string readBuffer;
    std::string method;
    std::vector<char> tempBuffer;

    off_t   file_offset;
    bool    connectionClose; 
    off_t   clientMaxBodySize;
    off_t   contentLength;
    timeval lastTimeRW;
    Server  server;

    unsigned int timeDifference() const;
    void    writeFromBuffer();
    void    writeFromFile();
public:
    Client();
    Client(const Client& o);
    bool    operator==(const Client& o) const;

    int     getSocketFd() const;
    std::string getRequest();
    Server& getServer();
    std::string getPostBody();
    bool    shouldBeClosed() const;

    void    setServer(Server server);
    void    setFd(int fd);
    void    setFileFd(int fd);
    void    setMethod(std::string& method);
    void    setContentLength(off_t length);
    void    setConnectionClose(bool close);

    bool    writeChunk();
    bool    readRequest();
    bool    readBody();
    void    storeResponse(const std::string& response);
    void    updateTimeout();

    ~Client();
};
