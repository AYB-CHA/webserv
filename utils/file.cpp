#include "file.hpp"

int utils::file::open(const char *file, int flags) {
    int fd = ::open(file, flags);
    if (fd < 0)
        throw std::runtime_error("open() failed.");
    return fd;
}

std::string utils::file::readN(unsigned int fd, unsigned int n) {
    char buffer[n + 1];
    int bytes = read(fd, buffer, n);
    if (bytes < 0)
        throw std::runtime_error("read() failed.");

    buffer[bytes] = '\0';
    std::string rstring(buffer);
    return rstring;
}