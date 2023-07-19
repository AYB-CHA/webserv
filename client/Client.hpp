#pragma once

#include "../server/Server.hpp"
#include <cerrno>
#include <exception>
#include <string>

class Client {
private:
    static const int read_buf_size;
    bool    requestRead;;
    int     socketFd;
    std::string writeBuffer;
    std::string readBuffer;
    std::string bodyBuffer;

    Server* server;
public:
    Client();
    Client(const Client& o);
    int     getSocketFd() const;
    bool    writeChunk();
    bool    readRequest(); // Reads request line and headers (no body)
    bool    operator==(const Client& o) const;
    std::string getRequest();
    Server&   getServer();
    bool    hasReadRequest() const;
    // void    setRequestRead(bool);
    void    setServer(Server *server);
    void    setFd(int fd);
    void    storeResponse(const std::string& response);
    class closeConnectionException : public std::exception {};
    ~Client();
};
