#pragma once

#include "../request/HttpRequest.hpp"
#include "../response/HttpResponseBuilder.hpp"
#include "../server/Server.hpp"

#include <sys/stat.h>

class Client;
class Mediator;
class RequestHandler {
  private:
    HttpResponseBuilder response;
    HttpRequest request;
    std::vector<Server> servers;
    int fd;
    bool handled;
    bool list_dir;

    //------- new attr
    std::string file;
    std::string extension;
    Location targetLoc;
    bool matchLocState;
    // struct stat data;

    // -------- new fun
    void checkConfAndAccess(Client &client);
    void listDirectory(Client &client);
    void fillContainer(std::string &container);
    void validMethod(const std::string &method, Client &c);
    bool isCGI(const std::string &extension);
    void fileRequested(Client &client, Mediator &mediator);

    Server &validServerName(std::string serverName);
    bool matchLocation(const std::string &endpoint, const Server &serv);
    // Location matchLocation(std::string endpoint, std::vector<Location>&
    // locations);
  public:
    RequestHandler();
    RequestHandler(const RequestHandler &other);
    RequestHandler &operator=(const RequestHandler &o);
    RequestHandler(HttpRequest &request, std::vector<Server> &servers);
    ~RequestHandler();

    int getFd();
    void setInitialized(bool handled);
    bool hasBeenInitialized() const;
    std::string getResponse();
    std::string getFileMimeType(const std::string &file_name) const;
    void init(Client &client);
    void handleFileRequest(Client &client, Mediator &mediator);
    void handleDELETE(Client &client);
    const std::string &getCgiPathFromExtension(const std::string &extension);
    void responseFromIndexes(Client &client, Mediator &mediator);
    Location &getLocation();
    bool matchedLocation();
    //
    bool isDirChecks(Client &clint);
    bool setIndexFile(const std::vector<std::string> &indexes);
    void DeleteFiles(const std::string &path, std::vector<std::string> &list);
};

#define forEach(type, iterable, name)                                          \
    for (type::iterator name = iterable.begin(); name != iterable.end(); name++)

#define forEachConst(type, iterable, name)                                     \
    for (type::const_iterator name = iterable.begin(); name != iterable.end(); \
         name++)
