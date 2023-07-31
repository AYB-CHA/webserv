#pragma once

#include <vector>
#include <algorithm>
#include "../server/Server.hpp"
#include "../client/Client.hpp"
#include "Mediator.hpp"

class Multiplexer {
private:
    typedef std::vector<Server*>::iterator SIter;
    typedef std::vector<Client*>::iterator CIter;
    std::vector<Server*> ready_servers;
    std::vector<Client*> cgi_pipes;
    std::vector<Client*> write_clients;
    std::vector<Client*> read_clients;
    std::vector<Server> servers;
    Mediator            mediator;

    void    acceptConnections();
    void    writeResponses();
    void    readFromPipes();
    void    readRequests();
public:
    Multiplexer(std::vector<Server> servers);
    void    run();
};
