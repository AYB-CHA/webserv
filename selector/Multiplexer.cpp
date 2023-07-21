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

Multiplexer::Multiplexer(std::vector<Server> servers) : mediator(servers) {}

void Multiplexer::run() {
    for (;;) {
        mediator.getBatch(ready_servers, read_clients, write_clients);

        for (SIter it = ready_servers.begin(); it != ready_servers.end();
             ++it) {
            int fd = accept(it->getSocketFd(), NULL, NULL);
            if (fd == -1) {
                if (errno != EWOULDBLOCK) {
                    throw std::runtime_error("accept() failed");
                }
                break;
            }
            mediator.addClient(fd, &*it);
        }

        for (CIter it = write_clients.begin(); it != write_clients.end(); ++it) {
            try {
                bool bufferisEmpty = it->writeChunk();
                mediator.updateClient(*it);
                if (!bufferisEmpty) {
                    CIter client =
                        std::find(read_clients.begin(), read_clients.end(), *it);
                    if (client != read_clients.end())
                        read_clients.erase(client);
                }
            } catch (Client::closeConnectionException& e) {
                mediator.removeClient(it->getSocketFd());
                close(it->getSocketFd());
            }
        }

        for (CIter it = read_clients.begin(); it != read_clients.end(); ++it) {
            try {
                if (!it->readRequest()) {
                    mediator.updateClient(*it);
                    continue;
                }

                std::string buffer = it->getRequest();
                // std::cout << buffer << std::endl;
                HttpRequest request; 
                HttpRequestParser parser(request, buffer);
                RequestHandler handler(request, it->getServer());// Pass the client
                it->storeResponse(handler.getResponse());
                mediator.updateClient(*it);
            } catch (HttpResponseException& e) {
                it->storeResponse(e.build());
                mediator.updateClient(*it);
            } catch (Client::closeConnectionException& e) {
                mediator.removeClient(it->getSocketFd());
                close(it->getSocketFd());
            }
        }
        mediator.filterClients();
    }
}
