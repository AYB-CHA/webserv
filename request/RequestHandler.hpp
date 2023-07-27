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
        bool matchLocation(std::string endpoint, const Server &serv, Location &target);

        int getFd();
        std::string getResponse();
        std::string getFileMimeType(const std::string &file_name) const;
};

#define forEach(type, iterable, name) \
    for (type::iterator name = iterable.begin(); name != iterable.end(); name++)

#define forEachConst(type, iterable, name) \
    for (type::const_iterator name = iterable.begin(); name != iterable.end(); name++)
