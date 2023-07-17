#pragma once
#include <iostream>

class HttpResponseException : public std::exception {
private:
  unsigned int status_code;

public:
  HttpResponseException(unsigned int status_code);
  std::string build() const;
  ~HttpResponseException() throw();
};