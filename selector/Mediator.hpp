#pragma once

#include "../client/Client.hpp"
#include "../server/Server.hpp"
#include "../selector/Selector.hpp"
#include <map>
#include <vector>

class Mediator {
private:
    std::map<int, Client>   fd_clients;
    std::map<int, Server>   fd_servers;
    Selector                selector;
public:
    Mediator(std::vector<Server>& initServers);
    void    addClient(int fd, const Server* server);
    void    removeClient(int fd);

    void    getBatch(std::vector<Server>&, std::vector<Client>& read, std::vector<Client>& write);
};
