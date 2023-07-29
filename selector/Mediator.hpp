#pragma once

#include "../client/Client.hpp"
#include "../server/Server.hpp"
#include "../selector/Selector.hpp"
#include <map>
#include <vector>
#include <iostream>

class Mediator {
private:
    std::map<int, Client>   fd_clients;
    std::map<int, Server>   fd_servers;
    std::vector<int>        fd_pipes;
    Selector                selector;
public:
    Mediator(std::vector<Server>& initServers);
    void    addClient(int fd, Server& server);
    void    removeClient(int fd);
    void    updateClient(Client client);
    void    filterClients();
    void    addCGI(int fd);
    void    removeCGI(int fd);

    void    getBatch(std::vector<Server>&, std::vector<Client>& read, std::vector<Client>& write, std::vector<Client>& pipes);
};
