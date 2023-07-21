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
    // std::string bodyBuffer; // Will be needed later for POST methods
    std::string readBuffer;
    std::string path;

    off_t   file_offset;
    bool    connectionClose; 
    timeval lastTimeRW;
    Server  server;

    unsigned int timeDifference() const;
public:
    Client();
    Client(const Client& o);
    bool    operator==(const Client& o) const;

    int     getSocketFd() const;
    std::string getRequest();
    Server& getServer();
    bool    shouldBeClosed() const;

    void    setServer(Server server);
    void    setFd(int fd);
    void    setFileFd(int fd);
    void    setConnectionClose(bool close);

    bool    writeChunk();
    bool    readRequest();
    void    storeResponse(const std::string& response);

    class closeConnectionException : public std::exception {};

    ~Client();
};
