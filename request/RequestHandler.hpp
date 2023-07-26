#pragma once

# include "../request/HttpRequest.hpp"
# include "../server/Server.hpp"
# include "../client/Client.hpp"
# include "../response/HttpResponseBuilder.hpp"

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

        std::string getResponse();
        int getFd();

        Server& validServerName(std::string serverName);
        Location matchLocation(std::string endpoint, std::vector<Location>& locations);
};
