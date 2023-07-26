#pragma once
#include <iostream>
#include <map>

class HttpResponseBuilder {
  protected:
    bool has_content_length;
    std::string http_version;
    int status_code;
    std::string status_code_phrase;
    std::map<std::string, std::string> headers;
    std::string body;

  public:
    HttpResponseBuilder();
    HttpResponseBuilder *setStatuscode(int);

    HttpResponseBuilder *setHeader(const std::string &, const std::string &);
    HttpResponseBuilder *pushBody(const std::string &);
    std::string build();
    const std::string &getStatusPhraseCode();
    ~HttpResponseBuilder();
};