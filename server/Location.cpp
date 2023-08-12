#include "Location.hpp"

Location::Location() {
    this->redirect = "";

    // from base class
    this->autoindex = true;
    this->upload_path = "/tmp";
    this->client_max_body_size = pow(2, 30);
}
Location::Location(const Location &o)
    : AContext(o), prefix(o.prefix), cgi_path(o.cgi_path),
      redirect(o.redirect) {}

Location &Location::operator=(const Location &o) {
    if (this == &o)
        return *this;

    this->root = o.root;
    this->upload_path = o.upload_path;
    this->allowed_methods = o.allowed_methods;
    this->index = o.index;
    this->error_page = o.error_page;
    this->client_max_body_size = o.client_max_body_size;
    this->autoindex = o.autoindex;

    this->prefix = o.prefix;
    this->cgi_path = o.cgi_path;
    this->redirect = o.redirect;

    return *this;
}

Location::~Location() {}

const std::vector<std::string> &Location::getPrefix(void) const {
    return this->prefix;
}
const std::map<std::string, std::string> &Location::getCgiPath(void) const {
    return this->cgi_path;
}
const std::string &Location::getRedirect(void) const { return this->redirect; }

void Location::setPrefix(std::vector<std::string> prefix) {
    this->prefix = prefix;
}
void Location::setCgiPath(std::string extension, std::string path) {
    this->cgi_path[extension] = path;
}
void Location::setRedirection(std::string redir) { this->redirect = redir; }
