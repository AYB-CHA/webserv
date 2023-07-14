#pragma once
#include "../main.hpp"
#include <vector>

class Core {
public:
  Core(std::vector<Server> servers);
  ~Core();
};