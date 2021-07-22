/*!
 * \file
 * \brief async IO operations
 */

#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include "reactor.h"


enum {
    CU_IN = EPOLLIN, ///< poll input events
    CU_OUT = EPOLLOUT ///< poll output events
};

/*!
 * Adds a file descriptor to polling list.
 * @param fd - file descriptor
 * @param opts - no zero opts
 *
 * Flags:
 * - #CU_IN - input events
 * - #CU_OUT - output events
 * @param reactor
 * @return on success, returns 0; on error, returns -1 and the error set appropriately.
 */
int cu_add_fd(int fd, uint32_t opts, cu_reactor_t reactor);

/*!
 * Async version of standard linux function *read*. See \code{.sh} man 2 read \endcode.
 */
ssize_t cu_read(int fd, void *buffer, size_t size, cu_reactor_t reactor);

/*!
 * Async version of standard linux function *write*. See \code{.sh} man 2 write \endcode.
 */
ssize_t cu_write(int fd, void *buffer, size_t size, cu_reactor_t reactor);

/*!
 * Async version of standard linux function *close*. See \code{.sh} man 2 close \endcode
 */
int cu_close(int fd, cu_reactor_t reactor);

/*!
 * Async version of linux system function *accept* fron <sys/socket.h>. See \code{.sh} man 2 accept \endcode
 */
int cu_accept(int fd, struct sockaddr *addr, socklen_t *len, cu_reactor_t reactor);
