#include <errno.h>

#include "coroutine_private.h"
#include "io.h"
#include "reactor_private.h"

int cu_add_fd(int fd, uint32_t opts, struct cu_reactor *reactor) {
    GArray *watchers = g_tree_lookup(reactor->fd_dict, &fd);
    if (watchers == NULL) {
        int *fdptr = malloc(sizeof(int));
        *fdptr = fd;
        watchers = g_array_new(FALSE, FALSE, sizeof(cu_coroutine_t));
        g_tree_insert(reactor->fd_dict, fdptr, watchers);
    } else {
        cu_seterr(EEXIST);
        return -1;
    }
    struct epoll_event fd_event;
    fd_event.data.fd = fd;
    fd_event.events = opts;
    if (epoll_ctl(reactor->epollfd, EPOLL_CTL_ADD, fd, &fd_event) == -1) {
        cu_seterr(errno);
        return -1;
    }
    return 0;
}

void wait_io_event(int fd, GArray *watchers, struct cu_reactor *reactor) {
    struct cu_coroutine *coro = cu_self(reactor);
    g_array_append_val(watchers, coro);

    ++reactor->files_cnt;
    reactor->caller = coro->id;
    getcontext(&coro->context);
    if (reactor->caller == coro->id) {
        setcontext(&reactor->context);
    }
    --reactor->files_cnt;
}

ssize_t cu_read(int fd, void *buffer, size_t size, struct cu_reactor *reactor) {
    GArray *watchers = g_tree_lookup(reactor->fd_dict, &fd);
    if (watchers == NULL) {
        cu_seterr(EINVAL);
        return -1;
    }
    wait_io_event(fd, watchers, reactor);
    int len;
    while ((len = read(fd, buffer, size)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            wait_io_event(fd, watchers, reactor);
        } else {
            cu_seterr(errno);
            return -1;
        }
    }
    return len;
}

ssize_t cu_write(int fd, void *buffer, size_t size, struct cu_reactor *reactor) {
    GArray *watchers = g_tree_lookup(reactor->fd_dict, &fd);
    if (watchers == NULL) {
        cu_seterr(EINVAL);
        return -1;
    }
    wait_io_event(fd, watchers, reactor);
    int len;
    while ((len = write(fd, buffer, size)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            wait_io_event(fd, watchers, reactor);
        } else {
            cu_seterr(errno);
            return -1;
        }
    }
    return len;
}

int cu_close(int fd, struct cu_reactor *reactor) {
    g_tree_remove(reactor->fd_dict, &fd);
    close(fd);
}

int cu_accept(int fd, struct sockaddr *addr, socklen_t *socklen, cu_reactor_t reactor) {
    GArray *watchers = g_tree_lookup(reactor->fd_dict, &fd);
    if (watchers == NULL) {
        cu_seterr(EINVAL);
        return -1;
    }
    wait_io_event(fd, watchers, reactor);
    int clientfd;
    while ((clientfd = accept(fd, addr, socklen)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            wait_io_event(fd, watchers, reactor);
        } else {
            cu_seterr(errno);
            return -1;
        }
    }
    return clientfd;
}
