#include "./Core.hpp"

Core::Core(std::vector<Server> &servers) {
    std::vector<Server>::iterator it;
    for (it = servers.begin(); it != servers.end(); ++it) {
        it->setUp();
    }
}

Core::~Core() {}
