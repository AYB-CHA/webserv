#include "Multiplexer.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <sys/errno.h>
#include <sys/socket.h>
#include "../request/HttpRequest.hpp"
#include "../request/HttpRequestParser.hpp"
#include "../response/HttpResponseException.hpp"
#include "../response/HttpResponseBuilder.hpp"

Multiplexer::Multiplexer(std::vector<Server> servers) : mediator(servers) {}

void Multiplexer::run() {
    for (;;) {
        mediator.getBatch(ready_servers, read_clients, write_clients);

        for (SIter it = ready_servers.begin(); it != ready_servers.end(); ++it) {
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
            bool bufferisEmpty = it->writeChunk();
            if (!bufferisEmpty) {
                CIter client = std::find(read_clients.begin(), read_clients.end(), it->getSocketFd());
                if (client != read_clients.end())
                    read_clients.erase(client);
            }
        }

        for (CIter it = read_clients.begin(); it != read_clients.end(); ++it) {
            bool doneReading;
            try {
                doneReading = it->readRequest(); //reads from its socket
            } catch (Client::closeConnectionException& e) {
                mediator.removeClient(it->getSocketFd());
                close(it->getSocketFd());
                continue;
            }
            if (doneReading == false) continue;

            std::string buffer = it->getRequest();
            HttpRequest request;
            try {
                HttpRequestParser parser(request, buffer);
            } catch (HttpResponseException& e) {
                it->storeResponse(e.build());
                continue;
            }

            // RequestHandler handler(request, it->getServer());
            // it->storeResponse(handler.build());
        }
    }
}
