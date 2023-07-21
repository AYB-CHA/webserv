#include "Mediator.hpp"
#include <cstring>
#include <stdexcept>
#include <unistd.h>

// Maybe set the timeout here
Mediator::Mediator(std::vector<Server>& init) {
    selector.setTimeout(5, 0);
    for (std::vector<Server>::iterator it = init.begin(); it != init.end(); ++it) {
        fd_servers[it->getSocketFd()] = *it;
        selector.pushFd(it->getSocketFd());
    }
}

void    Mediator::addClient(int fd, Server* server) {
    if (fd_clients.size() > FD_SETSIZE) {
        close(fd);
        return;
    }
    // std::cout << "Client has joined. id: " << fd << std::endl;
    Client client;
    client.setFd(fd);
    client.setServer(server);
    fd_clients[fd] = client;
    selector.pushFd(fd);
    // std::cout << "Num of clients: " << fd_clients.size() << std::endl;
}

void    Mediator::removeClient(int fd) {
    // std::cout << "Client has left. id: " << fd << std::endl;
    fd_clients.erase(fd);
    try {
        selector.popFd(fd);
    } catch (std::runtime_error& e) {
        //log the error for now
        std::cerr << e.what() << std::endl;
    }
}

void    Mediator::updateClient(Client client) {
    fd_clients[client.getSocketFd()] = client;
}

void    Mediator::filterClients() {
    // ok, just store the fds to remove, and then remove them one after one,
    // without using iterators here
    std::vector<int> toDelete;
    for (std::map<int, Client>::iterator it = fd_clients.begin(); it != fd_clients.end(); ++it) {
        if (it->second.shouldBeClosed()) {
            // std::cout << "this fd shouldBeClosed: " << it->second.getSocketFd() << std::endl;
            toDelete.push_back(it->second.getSocketFd());
            // removeClient(it->first);
            // close(it->first);
            //iterators are invalidated when you erase them 
        }
    }
    for (std::vector<int>::iterator it = toDelete.begin(); it != toDelete.end(); ++it) {
        removeClient(*it);
        close(*it);
    }
}

void    Mediator::getBatch(std::vector<Server>& servers, std::vector<Client>& rclients, std::vector<Client>& wclients) {
    servers.clear(); rclients.clear(); wclients.clear();

    if (selector.poll() == -1)
        throw std::runtime_error(std::string("select() failed: ")+ strerror(errno));

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
        if (fd_servers.find(fd) != fd_servers.end())
            throw std::runtime_error("server socket failed.");
        wclients.push_back(fd_clients[fd]);
    }
}
