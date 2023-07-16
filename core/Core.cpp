#include "./Core.hpp"

Core::Core(std::vector<Server> &servers) : servers(servers) {
    std::vector<Server>::iterator it;
    for (it = this->servers.begin(); it != this->servers.end(); it++) {
        it->setUp();
    }
}

Core::~Core() {}