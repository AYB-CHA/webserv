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

        //------- new attr
        std::string file;
        Location targetLoc;
        bool matchLocState;

        // -------- new fun
        void checkConfAndAccess(Client& client);
        void listDirectory ();
        void createContainer(std::string& container, std::string::size_type& index);
        void fillContainer(std::string& container, std::string::size_type& index);


        Server& validServerName(std::string serverName);
        bool matchLocation(std::string endpoint, const Server &serv);
        // Location matchLocation(std::string endpoint, std::vector<Location>& locations);
    public:
        RequestHandler();
        RequestHandler(const RequestHandler& other);
        RequestHandler& operator=(const RequestHandler& o);
        RequestHandler(HttpRequest &request, std::vector<Server>& servers);

        int getFd();
        void setInitialized(bool handled);
        bool hasBeenInitialized() const;
        std::string getResponse();
        std::string getFileMimeType(const std::string &file_name) const;
        void init(Client& client);
        void handleGET(Client& client, Mediator& mediator);
        bool handlePOST(Client& client, Mediator& mediator);




};

#define forEach(type, iterable, name) \
    for (type::iterator name = iterable.begin(); name != iterable.end(); name++)

#define forEachConst(type, iterable, name) \
    for (type::const_iterator name = iterable.begin(); name != iterable.end(); name++)
