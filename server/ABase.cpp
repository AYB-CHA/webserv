#include "ABase.hpp"

ABase::~ABase() {}

const std::string& ABase::getRoot(void) const {
	return this->root;
}
const std::string& ABase::getUploadPath(void) const {
	return this->upload_path;
}
const std::map<std::string, bool>&  ABase::getAllowedMethods(void) const {
	return this->allowed_methods;
}
const std::vector<std::string> &ABase::getIndex(void) const {
	return this->index;
}
const std::map<int, std::string>& ABase::getErrorPage(void) const {
	return this->error_page;
}
const long& ABase::getClientMaxBodySize(void) const {
	return this->client_max_body_size;
}

void ABase::setRoot(std::string root) {
	this->root = root;
}
void ABase::setUploadPath(std::string upload_path) {
	this->upload_path = upload_path;
}
void ABase::setAllowedMethods(std::string method) {
	this->allowed_methods[method] = true;
}
void ABase::setIndex(std::string index) {
	this->index.push_back(index);
}
void ABase::setErrorPage(int status_code, std::string path) {
	this->error_page[status_code] = path;
}
void ABase::setClientMaxBodySize(long max_body_size) {
	this->client_max_body_size = max_body_size;
}
