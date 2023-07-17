#pragma once
#include "HttpRequest.hpp"
#include <iostream>

class HttpRequestParser {
private:
  std::string request_string;
  HttpRequest &request;

public:
  HttpRequestParser(HttpRequest &request, std::string request_string);
  ~HttpRequestParser();

private:
  void parseRequestLine(const std::string &request_line);
  void parseHeaderLine(const std::string &header_line);

private:
  bool isValidMethod(std::string &method);
  std::string getNextLine();
};