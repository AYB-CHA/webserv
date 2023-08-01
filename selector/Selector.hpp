#pragma once

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <vector>
#include <cstring>

class Selector {
private:
    typedef std::vector<int>::iterator selIter;
    fd_set  read_set;
    fd_set  write_set;
    int     highest_fd;
    std::vector<int>    fds;
    std::vector<int>    readonly_fds;
    selIter rfd_pointer;
    selIter wfd_pointer;
    timeval timeout;
public:
    Selector();
    int poll();
    void pushFd(int fd, bool readOnly);
    void popFd(int fd);
    void    setTimeout(time_t sec, suseconds_t usec);
    int getWriteFd();
    int getReadFd();
};
