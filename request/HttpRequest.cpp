#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::HttpRequest(const HttpRequest& o): method(o.method), url(o.url), queries(o.queries),
headers(o.headers), body(o.body) {}
HttpRequest& HttpRequest::operator=(const HttpRequest& o) {
    if (this == &o) return *this;

    this->method = o.method;
    this->url = o.url;
    this->queries = o.queries;
    this->headers = o.headers;
    this->body = o.body;

    return *this;
}
HttpRequest::~HttpRequest() {}

const std::string &HttpRequest::getMethod() const { return this->method; }

const std::string& HttpRequest::getPathInfo() const { return this->url.pathinfo; }

const std::string &HttpRequest::getEndpoint() const { return this->url.endpoint; }
const std::string HttpRequest::getHeader(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator it;
    it = this->headers.find(key);

    if (it == this->headers.end())
        return std::string("");
    return it->second;
}

void HttpRequest::setMethod(std::string &method) { this->method = method; }

void HttpRequest::setPathInfo(const std::string& pathinfo) {
    this->url.pathinfo = pathinfo;
}

void HttpRequest::setEndpoint(const std::string &endpoint) {
    this->url.endpoint = endpoint;
}

void HttpRequest::setHeader(std::string &key, std::string &value) {
    this->headers.insert(std::make_pair(key, value));
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const {
    return this->headers;
}

void HttpRequest::pushBody(std::string &content) { this->body += content; }

const std::string &HttpRequest::getBody() { return this->body; }

const std::string &HttpRequest::getQueries() const { return this->queries; }

void HttpRequest::setQueries(const std::string &queries) {
    this->queries = queries;
}
