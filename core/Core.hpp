#pragma once
#include "../server/Server.hpp"

#include <vector>

class Core {
  private:
    std::vector<Server> servers;

  public:
    Core(std::vector<Server> &servers);
    ~Core();
};