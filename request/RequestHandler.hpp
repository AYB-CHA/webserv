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

        void handleIt();
    public:
        RequestHandler(HttpRequest &request, Client& client);

        std::string getResponse();
        int getFd();


        Location matchLocation(std::string endpoint, std::vector<Location>& locations);
};
