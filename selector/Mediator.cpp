#include "Mediator.hpp"
#include <stdexcept>

// Maybe set the timeout here
Mediator::Mediator(std::vector<Server>& init) {
    for (std::vector<Server>::iterator it = init.begin(); it != init.end(); ++it) {
        fd_servers[it->getSocketFd()] = *it;
        selector.pushFd(it->getSocketFd());
    }
}

void    Mediator::addClient(int fd) {
    // This line of code will probably need a few modifications, especially when it comes
    // To how the server is configured an how the client is supposed to receive the response
    fd_clients[fd] = Client();
    selector.pushFd(fd);
    // fd_servers[client.getSocketFd()] = client;
    // selector.pushFd(client.getSocketFd());
    throw std::runtime_error("addClient unimplemented!");
}

// When the multiplexer gets the batch, it will go through the servers and accept all connections
// And then it'll go through each client and handle them accordingly
void    Mediator::getBatch(std::vector<Server>& servers, std::vector<Client>& rclients, std::vector<Client>& wclients) {
    servers.clear(); rclients.clear(); wclients.clear();

    if (selector.poll() == -1)
        throw std::runtime_error("select() failed.");

    while (int fd = selector.getReadFd()) {
        if (fd == -1)
            break;
        if (fd_servers.find(fd) != fd_servers.end())
            servers.push_back(fd_servers[fd]);
        else
            rclients.push_back(fd_clients[fd]);
    }

    while (int fd = selector.getWriteFd()) {
        if (fd == -1)
            break;
        wclients.push_back(fd_clients[fd]);
    }
}
