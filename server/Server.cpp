#include "Server.hpp"
#include "AContext.hpp"

Server::Server() {
    this->port = 80;
    this->host = "127.0.0.1";

    // form base class
    this->autoindex = true;
    this->root = "./";
    this->upload_path = "/tmp/";
    this->client_max_body_size = pow(2, 30);
}

Server::Server(const Server &o)
    : AContext(o), port(o.port), host(o.host), server_names(o.server_names),
      redirect(o.redirect), location(o.location), host_add(o.host_add),
      socket_fd(o.socket_fd), host_add_len(o.host_add_len) {}

Server::~Server() {}

Server &Server::operator=(const Server &o) {

    if (this == &o)
        return *this;

    this->root = o.root;
    this->upload_path = o.upload_path;
    this->allowed_methods = o.allowed_methods;
    this->index = o.index;
    this->error_page = o.error_page;
    this->client_max_body_size = o.client_max_body_size;
    this->autoindex = o.autoindex;

    this->port = o.port;
    this->host = o.host;
    this->server_names = o.server_names;
    this->redirect = o.redirect;
    this->location = o.location;
    this->host_add = o.host_add;
    this->socket_fd = o.socket_fd;
    this->host_add_len = o.host_add_len;

    return *this;
}

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

int Server::getSocketFd() { return this->socket_fd; }
struct sockaddr_in &Server::getHostAdd() { return this->host_add; }
socklen_t &Server::getHostAddLength() { return this->host_add_len; }

void Server::setHost(std::string host) { this->host = host; }
void Server::setPort(int port) { this->port = port; }
void Server::setLocation(Location loc) { this->location.push_back(loc); }
void Server::setServerName(std::string server_name) {
    this->server_names.push_back(server_name);
}
void Server::setRedirect(std::string from, std::string to) {
    this->redirect[from] = to;
}
// #include <iostream>
void Server::setUp() {
    this->createSocket();
    this->setReUseAddressOption();
    this->bindAddress();
    this->listen();
}

void Server::createSocket() {
    this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socket_fd == -1)
        throw std::runtime_error("socket init: could't create socket for");
    fcntl(this->socket_fd, F_SETFL, O_NONBLOCK);
}
void Server::bindAddress() {
    this->host_add.sin_family = AF_INET;
    this->host_add.sin_port = htons(this->getPort());
    int inet_status = inet_pton(AF_INET, this->getHost().c_str(),
                                &(this->host_add.sin_addr.s_addr));
    if (inet_status == 0)
        throw std::runtime_error("socket binding: host " + this->getHost() +
                                 " is invalid.");
    else if (inet_status == -1)
        throw std::runtime_error("socket binding: inet_pton() failed.");

    this->host_add_len = sizeof(this->host_add);
    if (bind(this->socket_fd, (sockaddr *)&this->host_add, this->host_add_len))
        throw std::runtime_error("socket binding: could't bind the socket");
}

void Server::listen() {
    if (::listen(this->socket_fd, SOMAXCONN))
        throw std::runtime_error("socket init: could't listen to the socket");
}

void Server::setReUseAddressOption() {
    int value = 1;
    if (setsockopt(this->getSocketFd(), SOL_SOCKET, SO_REUSEPORT, &value,
                   sizeof(int)) < 0)
        throw std::runtime_error("socket init: setsockopt() failed.");
}
