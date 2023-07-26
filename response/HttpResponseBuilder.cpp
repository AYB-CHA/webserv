#include "HttpResponseBuilder.hpp"
#include "../utils/string.hpp"

HttpResponseBuilder::HttpResponseBuilder()
    : has_content_length(false), http_version("HTTP/1.1") {}

HttpResponseBuilder *HttpResponseBuilder::setStatuscode(int status_code) {
    this->status_code = status_code;
    switch (status_code) {
    case 200:
        this->status_code_phrase = "OK";
        break;
    case 404:
        this->status_code_phrase = "Not Found";
        break;
    case 500:
        this->status_code_phrase = "Internal Server Error";
        break;
    case 405:
        this->status_code_phrase = "Method Not Allowed";
        break;
    case 400:
        this->status_code_phrase = "Bad Request";
        break;
    default:
        this->status_code_phrase = "Must be implemented!!";
        break;
    }

    return this;
}

HttpResponseBuilder *HttpResponseBuilder::setHeader(const std::string &key,
                                                    const std::string &value) {
    this->headers[key] = value;
    if (key == "Content-Length")
        this->has_content_length = true;
    return this;
}

HttpResponseBuilder *HttpResponseBuilder::pushBody(const std::string &buffer) {
    this->body += buffer;
    return this;
}

std::string HttpResponseBuilder::build() {
    std::string response;
    response += this->http_version + " ";
    response += utils::string::fromInt(this->status_code) + " ";
    response += this->status_code_phrase + "\r\n";
    std::map<std::string, std::string>::iterator it;

    if (!this->has_content_length)
        this->setHeader("Content-Length",
                        utils::string::fromInt(this->body.length()));
    for (it = this->headers.begin(); it != this->headers.end(); ++it) {
        response += it->first + ": " + it->second + "\r\n";
    }

    response += "\r\n";
    response += this->body;
    return (response);
}

const std::string &HttpResponseBuilder::getStatusPhraseCode() {
    return this->status_code_phrase;
}

HttpResponseBuilder::~HttpResponseBuilder() {}
