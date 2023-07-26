#pragma once
#include <iostream>
#include <map>
#include <string>

class Mime {
  private:
    Mime();
    static Mime *self;
    std::map<std::string, std::string> types;
    std::string default_ext;

  public:
    static Mime *getInstance();
    const std::string &getMimeType(const std::string &extention);
    ~Mime();
};