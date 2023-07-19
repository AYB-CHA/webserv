#include "HttpRequestParser.hpp"
#include "../server/utils.hpp"
#include "../response/HttpResponseException.hpp"
#include "../utils/string.hpp"

#include <unistd.h>

HttpRequestParser::HttpRequestParser(HttpRequest &request, std::string request_string)
    : request_string(request_string), request(request) {
    std::string line = this->getNextLine();
    this->parseRequestLine(line);
    return; // temporary
    for (;;) {
        std::string line = this->getNextLine();
        std::cout << "line: " << line << std::endl;
        if (line == "")
            break;
        this->parseHeaderLine(line);
    }
}

HttpRequestParser::~HttpRequestParser() {}

void HttpRequestParser::parseRequestLine(const std::string &request_line) {

    //  method
    std::string::size_type first_space = request_line.find(' ');
    if (first_space == std::string::npos)
        throw HttpResponseException(400);
    std::string method = request_line.substr(0, first_space);
    if (!this->isValidMethod(method) || request_line[first_space + 1] == ' ')
        throw HttpResponseException(400);
    this->request.setMethod(method);
    // std::cout << "Method: " << method << std::endl;

    // end point.
    std::string::size_type second_space = request_line.find(' ', first_space + 1);
    std::string uri = request_line.substr(first_space + 1, second_space - first_space - 1);
    this->request.setEndpoint(uri);
    // std::cout << "endpoint: " << this->request.getEndpoint() << std::endl;

    // version...
    std::string http_version = request_line.substr(second_space + 1);
    utils::strTrim(http_version);
    if (http_version != "HTTP/1.1")
        throw HttpResponseException(400);
}

// add support for: Multiple message-header.
void HttpRequestParser::parseHeaderLine(const std::string &header_line) {
    std::string::size_type column = header_line.find(':');
    if (std::string::npos == column || header_line[column + 1] != ' ')
        throw HttpResponseException(400);
    std::string key = header_line.substr(0, column);
    std::string values = header_line.substr(column + 2);
    if (!utils::string::hasNoSpace(key) ||
        (values.length() && values[0] == ' '))
        throw HttpResponseException(400);
    this->request.setHeader(key, values);
}

bool HttpRequestParser::isValidMethod(std::string &method) {
    return method == "GET" || method == "POST" || method == "DELETE";
}

std::string HttpRequestParser::getNextLine() {
    std::string::size_type rc = this->request_string.find("\r\n");
    std::string line = this->request_string.substr(0, rc);
    this->request_string.erase(0, rc + 2);
    return line;
}
