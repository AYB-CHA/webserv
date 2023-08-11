#include "HttpResponseException.hpp"
#include "../utils/string.hpp"
#include "HttpResponseBuilder.hpp"

HttpResponseException::HttpResponseException(unsigned int status_code)
    : status_code(status_code) {}

HttpResponseException::HttpResponseException(unsigned int status_code, const std::string& path)
    : status_code(status_code),  path(path) {}


std::string HttpResponseException::build() const {
    HttpResponseBuilder builder;
    if (path.size())
        builder.setHeader("Location", path);
    return builder.setStatuscode(this->status_code)
        ->pushBody("<h1 style='text-align: center;'>" +
                   utils::string::fromInt(this->status_code) + " " +
                   builder.getStatusPhraseCode() + "<hr></h1>")
        ->build();
}

unsigned int HttpResponseException::getStatusCode() {
    return this->status_code;
}

HttpResponseException::~HttpResponseException() throw() {}
