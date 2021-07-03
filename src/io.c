#include <errno.h>

#include "io.h"

int cu_add_fd(int fd, cu_reactor_t *reactor) {
    struct epoll_event fd_event;
    fd_event.data.fd = fd;
    fd_event.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(reactor->epollfd, EPOLL_CTL_ADD, fd, &fd_event);
}

ssize_t cu_read(int fd, void *buffer, size_t size, cu_reactor_t *reactor) {
    GArray *watchers = g_tree_lookup(reactor->fd_dict, &fd);
    if (watchers == NULL) {
        int *fdptr = malloc(sizeof(int));
        *fdptr = fd;
        g_tree_insert(reactor->fd_dict, fd, g_array_new(FALSE, FALSE, sizeof(cu_coroutine_t)));
    }
    cu_coroutine_t *coro = cu_reactor_get_current_coro(reactor);
    g_array_append_val(watchers, coro);
    setcontext(&reactor->context);
    reactor->caller = coro->id;
    getcontext(&coro->context);
    if (reactor->caller == coro->id) {
        setcontext(&reactor->context);
    }
    return read(fd, buffer, size);
}

ssize_t cu_write(int fd, void *buffer, size_t size, cu_reactor_t *reactor) {
    GArray *watchers = g_tree_lookup(reactor->fd_dict, &fd);
    if (watchers == NULL) {
        int *fdptr = malloc(sizeof(int));
        *fdptr = fd;
        g_tree_insert(reactor->fd_dict, fd, g_array_new(FALSE, FALSE, sizeof(cu_coroutine_t)));
    }
    cu_coroutine_t *coro = cu_reactor_get_current_coro(reactor);
    g_array_append_val(watchers, coro);
    setcontext(&reactor->context);
    reactor->caller = coro->id;
    getcontext(&coro->context);
    if (reactor->caller == coro->id) {
        setcontext(&reactor->context);
    }
    return write(fd, buffer, size);
}