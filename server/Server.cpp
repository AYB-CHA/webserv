#include "../main.hpp"

Server::~Server() {}

const std::string &Server::getHost(void) const { return this->host; }
int Server::getPort(void) const { return this->port; }
const std::vector<Location> &Server::getLocation(void) const {
  return this->location;
}
const std::string &Server::getServerName(void) const {
  return this->server_name;
}
const std::string &Server::getRedirect(void) const { return this->redirect; }

void Server::setHost(std::string host) { this->host = host; }

void Server::setPort(int port) { this->port = port; }

void Server::setLocation(Location loc) { this->location.push_back(loc); }

void Server::setServerName(std::string server_name) {
  this->server_name = server_name;
}

void Server::setRedirect(std::string redirect) { this->redirect = redirect; }
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
  if (inet_pton(AF_INET, this->getHost().c_str(),
                &(this->host_add.sin_addr.s_addr)) <= 0) {
    // todo: throw a proper error.
    throw std::runtime_error(this->getHost() + " is invalid.");
  }
  this->host_add_len = sizeof(this->host_add);
  if (bind(this->socket_fd, (sockaddr *)&this->host_add, this->host_add_len))
    throw std::runtime_error("could't bind the socket");
}

void Server::listen() {
  if (::listen(this->socket_fd, SOMAXCONN))
    throw std::runtime_error("could't listen to the socket");
}

int Server::getSocketFd() { return this->socket_fd; }
struct sockaddr_in &Server::getHostAdd() { return this->host_add; }
socklen_t &Server::getHostAddLength() { return this->host_add_len; }