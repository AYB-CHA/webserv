#include "Multiplexer.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <sys/errno.h>
#include <sys/socket.h>
#include "../request/RequestHandler.hpp"
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
            std::cout << "I'm trying to write to: " << it->getSocketFd() << std::endl;
            bool bufferisEmpty = it->writeChunk();
            if (!bufferisEmpty) {
                CIter client = std::find(read_clients.begin(), read_clients.end(), *it);
                if (client != read_clients.end())
                    read_clients.erase(client);
            }
        }

        for (CIter it = read_clients.begin(); it != read_clients.end(); ++it) {
            // if (it->hasReadRequest() == false) {
                bool doneReading;
                try {
                    doneReading = it->readRequest(); //reads from its socket
                    mediator.updateClient(*it);
                    std::cout << "I'm still reading" << std::endl;
                } catch (Client::closeConnectionException& e) {
                    mediator.removeClient(it->getSocketFd());
                    close(it->getSocketFd());
                    continue;
                }
                if (doneReading == false) continue;
                // it->setRequestRead(true);
                // Once it's set to true, the only reason it'd keep going is that we still haven't
                // started sending the response to the client
             // } else {
            //     // it->readChunk(); // This is where you just continue reading the body when necessary
            // }

            std::string buffer = it->getRequest();
            std::cout << "request: " << buffer <<std::endl;
            HttpRequest request; 
            try {
                HttpRequestParser parser(request, buffer);
                RequestHandler handler(request, it->getServer());
                it->storeResponse(handler.getResponse());
                mediator.updateClient(*it);
                // std::cout << handler.getResponse();
            } catch (HttpResponseException& e) {
                it->storeResponse(e.build());
                std::cout << "Exception: " << e.what() << std::endl;
                continue;
            }

            // std::cout << request.getEndpoint() << std::endl;

            // std::cout <<"Hello" <<std::endl;
        }
    }
}
