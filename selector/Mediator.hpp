#pragma once

#include "../client/Client.hpp"
#include "../server/Server.hpp"
#include "../selector/Selector.hpp"
#include <map>

class Mediator {
private:
    std::map<int, Client>   clients;
    std::map<int, Client>   server;
    Selector                selector;
public:
    Mediator();
    void    getBatch(std::vector<Server>& servers, std::vector<Client>& clients);
};
