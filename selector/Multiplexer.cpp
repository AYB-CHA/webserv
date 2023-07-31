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

void    Multiplexer::acceptConnections() {
    for (SIter it = ready_servers.begin(); it != ready_servers.end();
    ++it) {
        int fd = accept((*it)->getSocketFd(), NULL, NULL);
        if (fd == -1) {
            if (errno != EWOULDBLOCK) {
                throw std::runtime_error("accept() failed");
            }
            break;
        }
        // std::cout << it->getLocation()[1].getRoot() << std::endl;
        mediator.addClient(fd, **it);
    }
}

void    Multiplexer::writeResponses() {
    for (CIter it = write_clients.begin(); it != write_clients.end(); ++it) {
        bool bufferisEmpty = (*it)->writeChunk();
        mediator.updateClient(**it);
        if (!bufferisEmpty) {
            CIter client =
                std::find(read_clients.begin(), read_clients.end(), *it);
            if (client != read_clients.end())
                read_clients.erase(client);
        }
    }
}

void    Multiplexer::readRequests() {
    for (CIter it = read_clients.begin(); it != read_clients.end(); ++it) {
        try {
            if (!(*it)->readRequest()) {
                mediator.updateClient(**it);
                continue;
            }
            (*it)->handleRequest(servers, mediator);
            mediator.updateClient(**it);
        } catch (HttpResponseException& e) {
            (*it)->storeResponse(e.build());
            mediator.updateClient(**it);
        }
    }
}

void    Multiplexer::readFromPipes() {
    for (CIter it = cgi_pipes.begin(); it != cgi_pipes.end(); ++it) {
        try {
            if ((*it)->readOutputCGI() == true) {//true meaning: it was done reading from the CGI
                mediator.removeCGI((*it)->getCgiFd());
            };
            mediator.updateClient(**it);
        } catch (HttpResponseException& e) {
            (*it)->storeResponse(e.build());
            mediator.updateClient(**it);
        }
    }
}

void Multiplexer::run() {
    for (;;) {
        mediator.getBatch(ready_servers, read_clients, write_clients, cgi_pipes);

        acceptConnections();
        readFromPipes();
        // for (CIter it = cgi_pipes.begin(); it != cgi_pipes.end(); ++it) {
        //     CIter client = std::find(read_clients.begin(), read_clients.end(), *it);
        //     if (client != read_clients.end())
        //         *client = *it;
        //     client = std::find(write_clients.begin(), write_clients.end(), *it);
        //     if (client != write_clients.end())
        //         *client = *it;
        // }

        writeResponses();
        readRequests();
        mediator.filterClients();
    }
}
