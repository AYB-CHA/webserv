#include <iostream>
#include "Selector.hpp"
#include <algorithm>
#include <stdexcept>
#include <sys/select.h>
#include <sys/types.h>

Selector::Selector() {}

void Selector::pushFd(int fd, selectType type) {
    fds.push_back(fd);
    switch (type) {
        case SEL_RDONLY: readonly_fds.push_back(fd); break;
        case SEL_WRONLY: writeonly_fds.push_back(fd); break;
        default: break;
    }
    highest_fd = *std::max_element(fds.begin(), fds.end());
}

void Selector::popFd(int fd) {
    selIter it = std::find(fds.begin(), fds.end(), fd);
    if (it == fds.end()) {
        throw std::runtime_error("Fd is not in the set.");
    }
    fds.erase(it);
    highest_fd = *std::max_element(fds.begin(), fds.end());
}

int Selector::poll() {
    rfd_pointer = fds.begin();
    wfd_pointer = fds.begin();

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    for (selIter it = fds.begin(); it != fds.end(); ++it) {
        if (std::find(writeonly_fds.begin(), writeonly_fds.end(), *it) == writeonly_fds.end())
            FD_SET(*it, &read_set);
        if (std::find(readonly_fds.begin(), readonly_fds.end(), *it) == readonly_fds.end())
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
