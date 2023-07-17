#pragma once

#include "../server/Server.hpp"
#include <cerrno>
#include <exception>
#include <string>

class Client {
private:
    static const int read_buf_size;
    int     socketFd;
    std::string writeBuffer;
    std::string readBuffer;
    std::string bodyBuffer;

    const Server* server;
public:
    Client(const Server* server);
    int     getSocketFd() const;
    bool    writeChunk();
    bool    readRequest(); // Reads request line and headers (no body)
    std::string getRequest();
    const Server&   getServer();
    // bool    readChunk(); // returns true if it's done reading
    void    storeResponse(const std::string& response);
    class closeConnectionException : public std::exception {};
    ~Client();
};
