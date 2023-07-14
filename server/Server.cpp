#include "Server.hpp"

Server::~Server() {}

const std::string& Server::getHost(void) const {
	return this->host;
}
int Server::getPort(void) const {
	return this->port;
}
const std::vector<Location>& Server::getLocation(void) const {
	return this->location;
}
const std::string& Server::getServerName(void) const {
	return this->server_name;
}
const std::string& Server::getRedirect(void) const {
	return this->redirect;
}

void Server::setHost(std::string host) {
	this->host = host;
}
void Server::setPort(int port) {
	this->port = port;
}
void Server::setLocation(Location loc) {
	this->location.push_back(loc);
}
void Server::setServerName(std::string server_name) {
	this->server_name = server_name;
}
void Server::setRedirect(std::string redirect) {
	this->redirect = redirect;
}
