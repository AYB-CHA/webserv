#include "Selector.hpp"
#include <algorithm>
#include <stdexcept>
#include <sys/select.h>

Selector::Selector() {
    FD_ZERO(&master_set);
}

void Selector::pushFd(int fd) {
    FD_SET(fd, &master_set);
    fds.push_back(fd);
    highest_fd = *std::max_element(fds.begin(), fds.end());
}

void Selector::popFd(int fd) {
    std::vector<int>::iterator it = std::find(fds.begin(), fds.end(), fd);
    if (it == fds.end()) {
        throw std::runtime_error("Fd is not in the set.");
    }
    FD_CLR(fd, &master_set);
}
