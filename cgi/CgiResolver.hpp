#pragma once
#include "../client/Client.hpp"
#include "../request/HttpRequest.hpp"
#include "../response/HttpResponseBuilder.hpp"
#include <iostream>
#include <map>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>

class CGIResolver {
  private:
    HttpRequest &request;
    HttpResponseBuilder &response;
    Client &client;

    std::string CGI_path;
    std::string CGI_file;
    std::string CGI_output;
    std::map<std::string, std::string> env;
    int read_pipes[2];
    int write_pipes[2];

  public:
    CGIResolver(const std::string &CGI_path, const std::string &CGI_file,
                HttpResponseBuilder &response, HttpRequest &request,
                Client &client);
    int getReadEnd() const;
    ~CGIResolver();

  private:
    void runCGI();
    void buildCGIEnv();
    bool validCGI() const;
};