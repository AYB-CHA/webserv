#pragma once
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

namespace utils {
class file {
  public:
    // will be needed latter.
    static int open(const char *file, int flags);
    static std::string readN(unsigned int fd, unsigned int n);
};
} // namespace utils