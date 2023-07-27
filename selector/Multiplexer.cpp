#include "Multiplexer.hpp"
#include "../request/HttpRequest.hpp"
#include "../request/HttpRequestParser.hpp"
#include "../request/RequestHandler.hpp"
#include "../response/HttpResponseBuilder.hpp"
#include "../response/HttpResponseException.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <sys/errno.h>
#include <sys/socket.h>

Multiplexer::Multiplexer(std::vector<Server> servers) : servers(servers), mediator(servers) {}

void    Multiplexer::acceptConnections(std::vector<Server>& ready_servers) {
    for (SIter it = ready_servers.begin(); it != ready_servers.end();
    ++it) {
        int fd = accept(it->getSocketFd(), NULL, NULL);
        if (fd == -1) {
            if (errno != EWOULDBLOCK) {
                throw std::runtime_error("accept() failed");
            }
            break;
        }
        // std::cout << it->getLocation()[1].getRoot() << std::endl;
        mediator.addClient(fd, *it);
    }
}

void    Multiplexer::writeResponses(std::vector<Client>& write_clients, std::vector<Client>& read_clients) {
    for (CIter it = write_clients.begin(); it != write_clients.end(); ++it) {
        bool bufferisEmpty = it->writeChunk();
        mediator.updateClient(*it);
        if (!bufferisEmpty) {
            CIter client =
                std::find(read_clients.begin(), read_clients.end(), *it);
            if (client != read_clients.end())
                read_clients.erase(client);
        }
    }
}

void    Multiplexer::readRequests(std::vector<Client>& read_clients) {
    for (CIter it = read_clients.begin(); it != read_clients.end(); ++it) {
        try {
            if (!it->readRequest()) {
                mediator.updateClient(*it);
                continue;
            }

            std::string buffer = it->getRequest();
            // std::cout << buffer << std::endl;
            HttpRequest request;
            // std::cout << it->getServer().getLocation()[1].getRoot() << std::endl;
            HttpRequestParser parser(request, buffer);
            RequestHandler handler(request, *it, servers);
            it->storeResponse(handler.getResponse());
            it->setFileFd(handler.getFd());
            mediator.updateClient(*it);
        } catch (HttpResponseException& e) {
            it->storeResponse(e.build());
            mediator.updateClient(*it);
        }
    }
}

void Multiplexer::run() {
    for (;;) {
        mediator.getBatch(ready_servers, read_clients, write_clients);

        acceptConnections(ready_servers);
        writeResponses(write_clients, read_clients);
        readRequests(read_clients);
        mediator.filterClients();
    }
}
