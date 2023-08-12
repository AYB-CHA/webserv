#pragma once

#include "../client/Client.hpp"
#include "../response/HttpResponseException.hpp"
#include "../server/Server.hpp"
#include "Mediator.hpp"
#include <algorithm>
#include <vector>

class Multiplexer {
  private:
    typedef std::vector<Server *>::iterator SIter;
    typedef std::vector<Client *>::iterator CIter;
    std::vector<Client> new_clients;
    std::vector<Server *> ready_servers;
    std::vector<Client *> cgi_inpipes;
    std::vector<Client *> cgi_outpipes;
    std::vector<Client *> write_clients;
    std::vector<Client *> read_clients;
    std::vector<Server> servers;
    Mediator mediator;

    void acceptConnections();
    void writeResponses();
    void readFromPipes();
    void readRequests();

  public:
    Multiplexer(std::vector<Server> servers);
    void run();
};
