#pragma once
#include <iostream>

class HttpResponseException : public std::exception {
  private:
    unsigned int status_code;
    std::string path;

  public:
    HttpResponseException(unsigned int status_code);
    HttpResponseException(unsigned int status_code, const std::string& path);
    std::string build() const;
    unsigned int getStatusCode();
    ~HttpResponseException() throw();
};
