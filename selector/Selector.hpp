#pragma once

#include <bits/types/struct_timeval.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <vector>

class Selector {
private:
    fd_set  master_set;
    fd_set  working_set;
    int     highest_fd;
    std::vector<int>    fds;
    std::vector<int>::iterator fd_pointer;
    timeval timeout;
public:
    Selector();
    int poll();
    void pushFd(int fd);
    void popFd(int fd);
    void    setTimeout();
    int getReadyFd();
};
