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
    if (qm_pos != std::string::npos) {
        URL url = parseURI(uri.substr(0, qm_pos));
        this->request.setEndpoint(url.endpoint);
        this->request.setPathInfo(url.pathinfo);
        this->request.setQueries(uri.substr(qm_pos + 1));
    } else {
        URL url = parseURI(uri);
        this->request.setEndpoint(url.endpoint);
        this->request.setPathInfo(url.pathinfo);
    }


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

    std::string::size_type index = uri.find("/");
    while (index != uri.npos) {
        char s = '/', d = '.';

        std::string base = uri.substr(index, uri.find("/", index + 1) - index);
        for (std::string::size_type i = base.size() -2; i > 0 && base.size() > 2; i--) {
            if (base[1] == d || base[base.size() -1] == d)
                break;
            char curr = base[i];
            char prev = base[i -1];
            char next = base[i +1];
            if (prev != s && prev != d && curr == d && next != s && next != d) {
                index++;
                url.endpoint = uri.substr(0, uri.find("/", index));
                url.pathinfo = uri.substr(url.endpoint.size());
                return url;
            }
        }
        index = uri.find("/", index + 1);
    }

    url.endpoint = uri;
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
