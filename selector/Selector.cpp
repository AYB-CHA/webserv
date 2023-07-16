#include "Selector.hpp"
#include <algorithm>
#include <stdexcept>
#include <sys/select.h>
#include <sys/types.h>

Selector::Selector() {
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
}

void Selector::pushFd(int fd) {
    FD_SET(fd, &read_set);
    FD_SET(fd, &write_set);
    fds.push_back(fd);
    highest_fd = *std::max_element(fds.begin(), fds.end());
}

void Selector::popFd(int fd) {
    selIter it = std::find(fds.begin(), fds.end(), fd);
    if (it == fds.end()) {
        throw std::runtime_error("Fd is not in the set.");
    }
    fds.erase(it);
    FD_CLR(fd, &read_set);
    FD_CLR(fd, &write_set);
}

int Selector::poll() {
    rfd_pointer = fds.begin();
    wfd_pointer = fds.begin();

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    for (selIter it = fds.begin(); it != fds.end(); ++it) {
        FD_SET(*it, &read_set);
        FD_SET(*it, &write_set);
    }

    return select(highest_fd + 1, &read_set, &write_set, NULL, &timeout);
}

int Selector::getWriteFd() {
    while (wfd_pointer != fds.end() && FD_ISSET(*wfd_pointer, &write_set) == false)
        ++wfd_pointer;
    if (wfd_pointer == fds.end())
        return -1;

    int fd = *wfd_pointer;
    ++wfd_pointer;
    return fd;
}

int Selector::getReadFd() {
    while (rfd_pointer != fds.end() && FD_ISSET(*rfd_pointer, &read_set) == false)
        ++rfd_pointer;
    if (rfd_pointer == fds.end())
        return -1;

    int fd = *rfd_pointer;
    ++rfd_pointer;
    return fd;
}

void Selector::setTimeout(time_t sec, suseconds_t usec) {
    timeout.tv_sec = sec;
    timeout.tv_usec = usec;
}
