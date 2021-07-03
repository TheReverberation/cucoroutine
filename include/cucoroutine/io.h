#pragma once

#include <unistd.h>

#include "reactor.h"

int cu_add_fd(int fd, cu_reactor_t *reactor);
ssize_t cu_read(int fd, void *buffer, size_t size, cu_reactor_t *reactor);
ssize_t cu_write(int fd, void *buffer, size_t size, cu_reactor_t *reactor);
