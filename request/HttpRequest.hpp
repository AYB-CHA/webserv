#pragma once
#include <iostream>
#include <map>

class HttpRequest {
  protected:
    std::string method;
    std::string endpoint;
    // todo: refactor the value to be a vector of strings,
    std::multimap<std::string, std::string> headers;
    std::string body;

  public:
    HttpRequest();
    ~HttpRequest();

  public:
    const std::string &getMethod() const;
    const std::string &getEndpoint() const;
    const std::multimap<std::string, std::string> &getHeaders() const;
    const std::string getHeader(const std::string &key) const;
    const std::string &getBody();

  public:
    void setMethod(std::string &method);
    void setEndpoint(std::string &endpoint);
    void setHeader(std::string &key, std::string &value);
    void pushBody(std::string &content);
};
