#pragma once

#include "../server/Server.hpp"
#include <cerrno>
#include <exception>
#include <string>
#include <sys/_types/_timeval.h>

class Client {
private:
    static const int read_buf_size;
    int     socketFd;
    std::string writeBuffer;
    std::string readBuffer;
    std::string bodyBuffer;
    std::string path;
    timeval     lastTimeRW;
    //add a timeout attribute to the client, that you check after every poll.
    //If it exceeds it, we close the connection and remove it from our map of clients

    Server* server;
public:
    Client();
    Client(const Client& o);
    bool    operator==(const Client& o) const;

    int     getSocketFd() const;
    std::string getRequest();
    Server&   getServer();

    void    setServer(Server *server);
    void    setFd(int fd);

    bool    writeChunk();
    bool    readRequest();
    void    storeResponse(const std::string& response);

    class closeConnectionException : public std::exception {};

    ~Client();
};
