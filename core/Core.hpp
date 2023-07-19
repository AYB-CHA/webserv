#pragma once
#include "../server/Server.hpp"

#include <vector>

class Core {
  public:
    Core(std::vector<Server> &servers);
    ~Core();
};
