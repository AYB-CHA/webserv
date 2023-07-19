#include "./Core.hpp"

// #include <iostream>
Core::Core(std::vector<Server> &servers) {
    std::vector<Server>::iterator it;
    for (it = servers.begin(); it != servers.end(); ++it) {
        it->setUp();
        // std::cout << it->getSocketFd() << std::endl;
    }
}

Core::~Core() {}
