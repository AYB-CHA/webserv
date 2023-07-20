#include "Client.hpp"
#if __linux__
#include <sys/sendfile.h>
#elif __APPLE__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif
int sendFile(int fileFd, int socketFd, off_t *offset, size_t count) {
#if __linux__
    return sendfile(fileFd, socketFd, offset, count);
#elif __APPLE__
    off_t len = count;
    int res = sendfile(fileFd, socketFd, *offset, &len, NULL, 0);
    *offset += len;
    if (res == 0 || res == -1)
        return res;
    return len;
#else
    static_assert(1, "");
#endif
}
