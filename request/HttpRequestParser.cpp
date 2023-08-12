#include "HttpRequestParser.hpp"
#include "../response/HttpResponseException.hpp"
#include "../server/utils.hpp"
#include "../utils/string.hpp"

#include <unistd.h>

HttpRequestParser::HttpRequestParser(HttpRequest &request,
                                     std::string request_string)
    : request_string(request_string), request(request) {
    std::string line = this->getNextLine();
    this->parseRequestLine(line);
    for (;;) {
        std::string line = this->getNextLine();
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

    // end point.
    std::string::size_type second_space =
        request_line.find(' ', first_space + 1);
    std::string uri =
        request_line.substr(first_space + 1, second_space - first_space - 1);
    std::string::size_type qm_pos = uri.find('?');
    // ?: remember to double check for nginx behavior on multiple question mark
    // ? eg: '/test?????abc=123'
    if (qm_pos != std::string::npos) {
        URL url = parseURI(uri);
        this->request.setEndpoint(url.path);
        // this->request.setPathInfo(url.pathinfo);
        // this->request.setEndpoint(uri.substr(0, qm_pos));
        this->request.setQueries(uri.substr(qm_pos + 1));
    } else
        this->request.setEndpoint(uri);

    // std::cout << "queries: " << this->request.getQueries() << std::endl;
    // std::cout << "endpoint: " << this->request.getEndpoint() << std::endl;

    // version...
    std::string http_version = request_line.substr(second_space + 1);
    utils::strTrim(http_version);
    if (http_version != "HTTP/1.1")
        throw HttpResponseException(400);
}

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

URL HttpRequestParser::parseURI(const std::string& uri) {
    URL url;

    // std::string::size_type index = uri.find("/");
    // while (index != uri.npos) {
    //     std::string binslash = uri.substr(index, uri.find("/", index + 1));
    //     for (int i = binslash.size() -2; i > 0; i++) {
    //         if (binslash[i - 1] != '/' && binslash[i] == '.' && binslash[i + 1] != '/') {
    //         }
    //     }
    //     index = uri.find("/", index + 1);
    // }
    url.path = uri;
    return url;
}

bool HttpRequestParser::isValidMethod(std::string &method) {
    return method == "GET" || method == "POST" || method == "DELETE";
}

std::string HttpRequestParser::getNextLine() {
    std::string::size_type rc = this->request_string.find("\r\n");
    if (rc == std::string::npos)
        throw HttpResponseException(400);
    std::string line = this->request_string.substr(0, rc);
    this->request_string.erase(0, rc + 2);
    return line;
}
