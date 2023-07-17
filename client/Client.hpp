#pragma once

#include "../server/Server.hpp"
#include <cerrno>
#include <string>

class Client {
private:
    int     socketFd;
    std::string writeBuffer;
    const Server& server;
public:
    Client(const Server& server);
    bool    writeChunk();
    int     getSocketFd() const;
    ~Client();
};
