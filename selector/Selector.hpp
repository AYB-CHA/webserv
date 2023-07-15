#pragma once

#include <bits/types/struct_timeval.h>
#include <bits/types/time_t.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <vector>
#include <cstring>

class Selector {
private:
    typedef std::vector<int>::iterator seliter;
    fd_set  master_set;
    fd_set  working_set;
    int     highest_fd;
    std::vector<int>    fds;
    seliter fd_pointer;
    timeval timeout;
public:
    Selector();
    int poll();
    void pushFd(int fd);
    void popFd(int fd);
    void    setTimeout(time_t sec, suseconds_t usec);
    int getReadyFd();
};
