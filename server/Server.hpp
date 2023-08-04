#pragma once
#include "Location.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class Server : public AContext {
  private:
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::map<std::string, std::string> redirect;
    std::vector<Location> location;

    struct sockaddr_in host_add;
    int socket_fd;
    socklen_t host_add_len;

    void createSocket();
    void bindAddress();
    void listen();

  public:
    Server();
    Server(const Server& o);
    Server& operator=(const Server& o);
    ~Server();

    const std::string &getHost(void) const;
    int getPort(void) const;
    const std::vector<std::string> &getServerNames(void) const;
    const std::map<std::string, std::string> &getRedirect(void) const;
    const std::vector<Location> &getLocation(void) const;

    void setHost(std::string host);
    void setPort(int port);
    void setServerName(std::string server_name);
    void setRedirect(std::string form, std::string to);
    void setLocation(Location loc);
    void setReUseAddressOption();
    struct sockaddr_in &getHostAdd();
    socklen_t &getHostAddLength();
    int getSocketFd();

    void setUp();
};
