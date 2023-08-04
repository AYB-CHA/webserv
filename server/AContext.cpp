#include "AContext.hpp"

AContext::AContext() {
    this->allowed_methods["GET"] = false;
    this->allowed_methods["POST"] = false;
    this->allowed_methods["DELETE"] = false;

}
AContext::AContext(const AContext& o) : root(o.root), upload_path(o.upload_path), allowed_methods(o.allowed_methods),
    index(o.index), error_page(o.error_page), client_max_body_size(o.client_max_body_size), autoindex(o.autoindex){}
AContext::~AContext() {}

const std::string& AContext::getRoot(void) const {
	return this->root;
}
const std::string& AContext::getUploadPath(void) const {
	return this->upload_path;
}
const std::map<std::string, bool>&  AContext::getAllowedMethods(void) const {
	return this->allowed_methods;
}
const std::vector<std::string> &AContext::getIndex(void) const {
	return this->index;
}
const std::map<int, std::string>& AContext::getErrorPage(void) const {
	return this->error_page;
}
const long& AContext::getClientMaxBodySize(void) const {
	return this->client_max_body_size;
}
bool AContext::getAutoindex(void) const {
	return this->autoindex;
}

void AContext::setRoot(std::string root) {
	this->root = root;
}
void AContext::setUploadPath(std::string upload_path) {
	this->upload_path = upload_path;
}
void AContext::setAllowedMethods(std::string method) {
	this->allowed_methods[method] = true;
}
void AContext::setIndex(std::string index) {
	this->index.push_back(index);
}
void AContext::setErrorPage(int status_code, std::string path) {
	this->error_page[status_code] = path;
}
void AContext::setClientMaxBodySize(long max_body_size) {
	this->client_max_body_size = max_body_size;
}
void AContext::setAutoindex(bool autoindex) {
	this->autoindex = autoindex;
}