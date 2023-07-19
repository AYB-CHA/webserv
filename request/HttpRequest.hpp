#pragma once
#include <iostream>
#include <map>
#include <vector>

class HttpRequest {
  protected:
    std::string method;
    std::string endpoint;
    std::string queries;
    std::multimap<std::string, std::string> headers;
    std::string body;

  public:
    HttpRequest();
    ~HttpRequest();

  public:
    const std::string &getMethod() const;
    const std::string &getEndpoint() const;
    const std::string &getQueries() const;
    const std::multimap<std::string, std::string> &getHeaders() const;
    const std::string getHeader(const std::string &key) const;
    const std::string &getBody();

  public:
    void setQueries(const std::string &queries);
    void setMethod(std::string &method);
    void setEndpoint(const std::string &endpoint);
    void setHeader(std::string &key, std::string &value);
    void pushBody(std::string &content);
};
