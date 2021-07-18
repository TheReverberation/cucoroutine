#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include "reactor.h"

enum {
    CU_IN = EPOLLIN,
    CU_OUT = EPOLLOUT
};

int cu_add_fd(int fd, uint32_t opts, cu_reactor_t reactor);
ssize_t cu_read(int fd, void *buffer, size_t size, cu_reactor_t reactor);
ssize_t cu_write(int fd, void *buffer, size_t size, cu_reactor_t reactor);
int cu_close(int fd, cu_reactor_t reactor);
int cu_accept(int fd, struct sockaddr *addr, socklen_t *len, cu_reactor_t reactor);
