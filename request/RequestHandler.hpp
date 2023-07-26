#pragma once

#include "../client/Client.hpp"
#include "../request/HttpRequest.hpp"
#include "../response/HttpResponseBuilder.hpp"
#include "../server/Server.hpp"

class RequestHandler {
  private:
    HttpResponseBuilder response;
    HttpRequest request;
    Client client;
    int fd;

    void handleIt();

  public:
    RequestHandler(HttpRequest &request, Client &client);

    std::string getResponse();
    int getFd();

    Location matchLocation(std::string endpoint,
                           std::vector<Location> &locations);
    std::string getFileMimeType(const std::string &file_name) const;
};
