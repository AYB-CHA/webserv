#pragma once

#include "../request/HttpRequest.hpp"
#include "../response/HttpResponseBuilder.hpp"
#include "../server/Server.hpp"

class Client;
class Mediator;
class RequestHandler {
  private:
        HttpResponseBuilder response;
        HttpRequest request;
        std::vector<Server> servers;
        int fd;
        bool    handled;


        Server& validServerName(std::string serverName);
        Location matchLocation(std::string endpoint, std::vector<Location>& locations);
    public:
        RequestHandler();
        RequestHandler(const RequestHandler& other);
        RequestHandler(HttpRequest &request, std::vector<Server>& servers);

        int getFd();
        void    setInitialized(bool handled);
        bool    hasBeenInitialized() const;
        std::string getResponse();
        std::string getFileMimeType(const std::string &file_name) const;
        void init(Client& client);
        void handleGET(Client& client, Mediator& mediator);
        bool handlePOST(Client& client, Mediator& mediator);
};
