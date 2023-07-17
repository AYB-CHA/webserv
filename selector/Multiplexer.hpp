#pragma once

#include <vector>
#include "../server/Server.hpp"
#include "../client/Client.hpp"
#include "Mediator.hpp"
class Multiplexer {
private:
    typedef std::vector<Server>::iterator SIter;
    typedef std::vector<Client>::iterator CIter;
    std::vector<Server> ready_servers;
    std::vector<Client> write_clients;
    std::vector<Client> read_clients;
    Mediator            mediator;
public:
    Multiplexer(std::vector<Server> servers);
    void    run();
};
