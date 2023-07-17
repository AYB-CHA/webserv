#pragma once

#include "../server/Server.hpp"
#include <cerrno>
#include <string>

class Client {
private:
    int     socketFd;
    std::string writeBuffer;

    std::string readBuffer;

    const Server& server;
public:
    Client(const Server& server);
    int     getSocketFd() const;
    bool    writeChunk();
    bool    readRequest(); // Reads request line and headers (no body)
    // bool    readChunk(); // returns true if it's done reading
    void    storeResponse(const std::string& response);
    ~Client();
};
