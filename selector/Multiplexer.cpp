#include "Multiplexer.hpp"
#include "../request/HttpRequest.hpp"
#include "../request/HttpRequestParser.hpp"
#include "../request/RequestHandler.hpp"
#include "../response/HttpResponseBuilder.hpp"
#include "../response/HttpResponseException.hpp"
#include "../utils/string.hpp"
#include <algorithm>
#include <csignal>
#include <cstddef>
#include <stdexcept>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/socket.h>

Multiplexer::Multiplexer(std::vector<Server> servers)
    : servers(servers), mediator(servers) {}

void Multiplexer::acceptConnections() {
    for (SIter it = ready_servers.begin(); it != ready_servers.end(); ++it) {
        int fd = accept((*it)->getSocketFd(), NULL, NULL);
        new_clients.push_back(Client(fd, **it));
    }
}

void Multiplexer::writeResponses() {
    for (CIter it = write_clients.begin(); it != write_clients.end(); ++it) {
        bool bufferisEmpty = (*it)->writeChunk();
        if (!bufferisEmpty) {
            CIter client =
                std::find(read_clients.begin(), read_clients.end(), *it);
            if (client != read_clients.end())
                read_clients.erase(client);
        }
    }
}

void Multiplexer::readRequests() {
    for (CIter it = read_clients.begin(); it != read_clients.end(); ++it) {
        try {
            if (!(*it)->readRequest()) {
                continue;
            }
            (*it)->handleRequest(servers, mediator);
        } catch (HttpResponseException &e) {
            (*it)->showErrorPage(e);
        }
    }
}

void Multiplexer::readFromPipes() {
    for (CIter it = cgi_inpipes.begin(); it != cgi_inpipes.end(); ++it) {
        try {
            if ((*it)->readOutputCGI() == true) {
                mediator.removeReadCGI((*it)->getCgiReadFd());
                (*it)->setCgiReadFd(-1);
            };
        } catch (HttpResponseException &e) {
            (*it)->showErrorPage(e);
            if ((*it)->getCgiReadFd() != -1)
                mediator.removeReadCGI((*it)->getCgiReadFd());
            (*it)->setCgiReadFd(-1);
        }
    }
}

void Multiplexer::writeToPipes() {
    for (CIter it = cgi_outpipes.begin(); it != cgi_outpipes.end(); ++it) {
        try {
            if ((*it)->writeBodyCGI() == true) {
                mediator.removeWriteCGI((*it)->getCgiWriteFd());
                (*it)->setCgiWriteFd(-1);
            };
        } catch (HttpResponseException &e) {
            (*it)->showErrorPage(e);
            if ((*it)->getCgiWriteFd() != -1)
                mediator.removeWriteCGI((*it)->getCgiWriteFd());
            (*it)->setCgiWriteFd(-1);
            if ((*it)->getCgiReadFd() != -1)
                mediator.removeReadCGI((*it)->getCgiReadFd());
            (*it)->setCgiReadFd(-1);
        }
    }
}

void Multiplexer::run() {
    try {
        for (;;) {
            mediator.getBatch(ready_servers, read_clients, write_clients,
                              cgi_inpipes, cgi_outpipes);

            acceptConnections();
            writeToPipes();
            readFromPipes();
            writeResponses();
            readRequests();
            mediator.filterClients();
            for (std::vector<Client>::iterator it = new_clients.begin();
                 it != new_clients.end(); ++it) {
                mediator.addClient(it->getSocketFd(), it->getServer());
            }
            new_clients.clear();
        }
    } catch (const std::bad_alloc& e) {
        mediator.clearAll();
        throw std::bad_alloc();
    }
}
