#include "Multiplexer.hpp"
#include "../request/HttpRequest.hpp"
#include "../request/HttpRequestParser.hpp"
#include "../request/RequestHandler.hpp"
#include "../response/HttpResponseBuilder.hpp"
#include "../response/HttpResponseException.hpp"
#include "../response/Mime.hpp"
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
        if (fd == -1) {
            if (errno != EWOULDBLOCK) {
                throw std::runtime_error("accept() failed");
            }
            break;
        }
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
            // (*it)->storeResponse(e.build());
            this->showErrorPage(e, *it);
        }
    }
}

void Multiplexer::readFromPipes() {
    for (CIter it = cgi_pipes.begin(); it != cgi_pipes.end(); ++it) {
        try {
            if ((*it)->readOutputCGI() == true) {
                mediator.removeCGI((*it)->getCgiFd());
                (*it)->setMethod("GET");
                (*it)->setCgiFd(-1);
            };
        } catch (HttpResponseException &e) {
            // (*it)->storeResponse(e.build());
            this->showErrorPage(e, *it);
        }
    }
}

void Multiplexer::showErrorPage(HttpResponseException &e,
                                Client *client) const {
    const std::map<int, std::string> &error_pages =
        (client->getRequestHandler().matchedLocation())
            ? client->getRequestHandler().getLocation().getErrorPage()
            : client->getServer().getErrorPage();
    if (error_pages.find(e.getStatusCode()) != error_pages.end()) {
        std::string file_path = error_pages.at(e.getStatusCode());
        int fd = open(file_path.c_str(), O_RDONLY);
        if (fd != -1) {
            HttpResponseBuilder builder;
            std::string extention =
                file_path.substr(file_path.find_last_of('.') + 1);
            struct stat data;
            fstat(fd, &data);
            std::cout << "file size: " << data.st_size << std::endl;
            builder.setStatuscode(200)
                ->setHeader("Content-Type",
                            Mime::getInstance()->getMimeType(extention))
                ->setHeader("Content-Length",
                            ::utils::string::fromInt(data.st_size));

            client->storeResponse(builder.build());
            client->setFileFd(fd);
            return;
        }
    }
    client->storeResponse(e.build());
}

void Multiplexer::run() {
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    for (;;) {
        mediator.getBatch(ready_servers, read_clients, write_clients,
                          cgi_pipes);

        acceptConnections();
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
}
