#pragma once

#include "../server/Server.hpp"
#include <sys/time.h>
#include <cerrno>
#include <exception>
#include <string>

class Client {
private:
    static const int read_buf_size;
    int     socketFd;
    int     bodyFd;
    std::string writeBuffer;
    std::string bodyBuffer;
    std::string readBuffer;
    std::string path;
    bool    connectionClose; 
    timeval     lastTimeRW;
    Server* server;

    unsigned int timeDifference();
public:
    Client();
    Client(const Client& o);
    bool    operator==(const Client& o) const;

    int     getSocketFd() const;
    std::string getRequest();
    Server&   getServer();
    bool    shouldBeClosed() const;

    void    setServer(Server *server);
    void    setFd(int fd);

    bool    writeChunk();
    bool    readRequest();
    void    storeResponse(const std::string& response);

    class closeConnectionException : public std::exception {};

    ~Client();
};
