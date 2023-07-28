#include "Location.hpp"

Location::Location() {
    // from base class
    this->autoindex = true;
    this->root = "";
    this->upload_path = "/upload/clientFile/";
    this->client_max_body_size = pow(2, 30);
}
Location::Location(const Location& o) : ABase(o), prefix(o.prefix), cgi_path(o.cgi_path) {}
Location::~Location() {}

const std::vector<std::string>& Location::getPrefix(void) const {
	return this->prefix;
}
const std::map<std::string, std::string>& Location::getCgiPath(void) const {
	return this->cgi_path;
}

void Location::setPrefix(std::vector<std::string> prefix) {
	this->prefix = prefix;
}
void Location::setCgiPath(std::string extension, std::string path) {
	this->cgi_path[extension] = path;
}
