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

        std::vector<Server> servers;

        void handleIt();

    public:
        RequestHandler(HttpRequest &request, Client& client, std::vector<Server> servers);

        Server& validServerName(std::string serverName);
        Location matchLocation(std::string endpoint, std::vector<Location>& locations);

        int getFd();
        std::string getResponse();
        std::string getFileMimeType(const std::string &file_name) const;
};
