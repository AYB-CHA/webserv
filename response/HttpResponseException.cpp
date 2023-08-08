#include "HttpResponseException.hpp"
#include "../utils/string.hpp"
#include "HttpResponseBuilder.hpp"

HttpResponseException::HttpResponseException(unsigned int status_code)
    : status_code(status_code) {}

std::string HttpResponseException::build() const {
    HttpResponseBuilder builder;
    return builder.setStatuscode(this->status_code)
        ->pushBody("<h1 style='text-align: center;'>" +
                   utils::string::fromInt(this->status_code) + " " +
                   builder.getStatusPhraseCode() + "<hr></h1>")
        ->build();
}

HttpResponseException::~HttpResponseException() throw() {}