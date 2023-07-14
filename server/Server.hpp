#pragma once
#include "../main.hpp"
#include "Location.hpp"
#include <string>
#include <vector>

class Server : public ABase {
  private:
    int port;
    std::string host;
    std::string server_name;
    std::string redirect;
    std::vector<Location> location;

    struct sockaddr_in host_add;
    int socket_fd;
    socklen_t host_add_len;

  public:
    ~Server();

    const std::string &getHost(void) const;
    int getPort(void) const;
    const std::string &getServerName(void) const;
    const std::string &getRedirect(void) const;
    const std::vector<Location> &getLocation(void) const;

    void setHost(std::string host);
    void setPort(int port);
    void setServerName(std::string root);
    void setRedirect(std::string redirect);
    void setLocation(Location loc);

    void setUp();

  private:
    void createSocket();
    void bindAddress();
    void listen();

  public:
    int getSocketFd();
    struct sockaddr_in &getHostAdd();
    socklen_t &getHostAddLength();
};
