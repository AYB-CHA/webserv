#include "Selector.hpp"
#include <algorithm>
#include <stdexcept>
#include <sys/select.h>
#include <sys/types.h>

Selector::Selector() {
    FD_ZERO(&master_set);
}

void Selector::pushFd(int fd) {
    FD_SET(fd, &master_set);
    fds.push_back(fd);
    highest_fd = *std::max_element(fds.begin(), fds.end());
}

void Selector::popFd(int fd) {
    selIter it = std::find(fds.begin(), fds.end(), fd);
    if (it == fds.end()) {
        throw std::runtime_error("Fd is not in the set.");
    }
    fds.erase(it);
    FD_CLR(fd, &master_set);
}

int Selector::poll() {
    fd_pointer = fds.begin();

    std::memcpy(&working_set, &master_set, sizeof(fd_set));
    return select(highest_fd + 1, &working_set, NULL, NULL, &timeout);
}

int Selector::getReadyFd() {
    while (fd_pointer != fds.end() && FD_ISSET(*fd_pointer, &working_set) == false)
        ++fd_pointer;
    if (fd_pointer == fds.end())
        return -1;

    int fd = *fd_pointer;
    ++fd_pointer;
    return fd;
}

void Selector::setTimeout(time_t sec, suseconds_t usec) {
    timeout.tv_sec = sec;
    timeout.tv_usec = usec;
}
