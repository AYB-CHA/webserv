#pragma once

# include "../request/HttpRequest.hpp"
# include "../server/Server.hpp"
# include "../response/HttpResponseBuilder.hpp"

class RequestHandler {
    private:
        HttpResponseBuilder response;
        HttpRequest request;
        Server server;

        void handleIt();


    public:
        RequestHandler(HttpRequest &request, Server &server);

        std::string getResponse();

};
