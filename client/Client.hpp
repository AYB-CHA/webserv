#pragma once

#include "../server/Server.hpp"

class Client {
private:
    const Server& server;
public:
    Client(const Server& server);
    ~Client();
};
