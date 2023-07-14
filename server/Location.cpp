#include "Location.hpp"

Location::~Location() {}

const std::string& Location::getPrefix(void) const {
	return this->prefix;
}
bool Location::getAutoindex(void) const {
	return this->autoindex;
}
const std::map<std::string, std::string>& Location::getCgiPath(void) const {
	return this->cgi_path;
}

void Location::setPrefix(std::string prefix) {
	this->prefix = prefix;
}
void Location::setAutoindex(bool autoindex) {
	this->autoindex = autoindex;
}
void Location::setCgiPath(std::string extension, std::string path) {
	this->cgi_path[extension] = path;
}
