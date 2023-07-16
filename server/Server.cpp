#include "./Server.hpp"

Server::~Server() {}

const std::string &Server::getHost(void) const { return this->host; }

int Server::getPort(void) const { return this->port; }
const std::vector<Location> &Server::getLocation(void) const {
    return this->location;
}
const std::vector<std::string> &Server::getServerNames(void) const {
    return this->server_names;
}
const std::map<std::string, std::string> &Server::getRedirect(void) const {
    return this->redirect;
}

void Server::setHost(std::string host) { this->host = host; }

void Server::setPort(int port) { this->port = port; }

void Server::setLocation(Location loc) { this->location.push_back(loc); }

void Server::setUp() {
    this->createSocket();
    this->bindAddress();
    this->listen();
}

void Server::createSocket() {
    this->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->socket_fd == -1)
        throw std::runtime_error("could't create socket");
}
void Server::bindAddress() {
    this->host_add.sin_family = AF_INET;
    this->host_add.sin_port = htons(this->getPort());
    int inet_status = inet_pton(AF_INET, this->getHost().c_str(),
                                &(this->host_add.sin_addr.s_addr));
    if (inet_status == 0)
        throw std::runtime_error("host " + this->getHost() + " is invalid.");
    else if (inet_status == -1)
        throw std::runtime_error("inet_pton() failed.");

    this->host_add_len = sizeof(this->host_add);
    if (bind(this->socket_fd, (sockaddr *)&this->host_add, this->host_add_len))
        throw std::runtime_error("could't bind the socket");
}

void Server::listen() {
    if (::listen(this->socket_fd, SOMAXCONN))
        throw std::runtime_error("could't listen to the socket");
}

void Server::setServerName(std::string server_name) {
    this->server_names.push_back(server_name);
}
void Server::setRedirect(std::string from, std::string to) {
    this->redirect[from] = to;
}

int Server::getSocketFd() { return this->socket_fd; }
struct sockaddr_in &Server::getHostAdd() { return this->host_add; }
socklen_t &Server::getHostAddLength() { return this->host_add_len; }