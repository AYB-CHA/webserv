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
            mediator.addClient(fd);
        }

        for (CIter it = write_clients.begin(); it != write_clients.end(); ++it) {
            // bool bufferisEmpty = it->writeChunk();
            // if (!bufferisEmpty) {
            //     CIter client = std::find(it->getSocketFd(), read_clients.begin(), read_clients.end());
            //     if (client != read_clients.end())
            //         read_clients.erase(client);
            // }
        }

        for (CIter it = read_clients.begin(); it != read_clients.end(); ++it) {
            // std::string buffer = it->read(); //reads from its socket
            //
            // if it->read() returns EOF we close the connection (and tell the mediator to remove
            // the fd as well);
            // HttpRequest request;
            // try {
            //     HttpRequestParser parser(request, buffer);
            // } catch (HttpResponseException& e) {
            //     it->storeResponse(e.build());
            //     continue;
            // }

            // RequestHandler handler(request, it->getServer());
            // it->storeResponse(handler.build());
        }
    }
}
