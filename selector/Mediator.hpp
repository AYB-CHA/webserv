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
    std::vector<int>        fd_readpipes;
    std::vector<int>        fd_writepipes;
    Selector                selector;
public:
    Mediator(std::vector<Server>& initServers);
    void    addClient(int fd, Server& server);
    void    removeClient(int fd);
    void    filterClients();
    void    clearAll();
    void    addReadCGI(int fd);
    void    addWriteCGI(int fd);
    void    removeReadCGI(int fd);
    void    removeWriteCGI(int fd);

    void    getBatch(std::vector<Server*>&,std::vector<Client*>&,std::vector<Client*>&,std::vector<Client*>&, std::vector<Client*>&);
};
