#include <errno.h>

#include "coroutine_private.h"
#include "io.h"
#include "reactor_private.h"

int cu_add_fd(int fd, uint32_t opts, struct cu_reactor *reactor) {
    struct epoll_event fd_event;
    fd_event.data.fd = fd;
    fd_event.events = opts;
    return epoll_ctl(reactor->epollfd, EPOLL_CTL_ADD, fd, &fd_event);
}

ssize_t cu_read(int fd, void *buffer, size_t size, struct cu_reactor *reactor) {
    GArray *watchers = g_tree_lookup(reactor->fd_dict, &fd);
    if (watchers == NULL) {
        int *fdptr = malloc(sizeof(int));
        *fdptr = fd;
        g_tree_insert(reactor->fd_dict, fdptr, g_array_new(FALSE, FALSE, sizeof(cu_coroutine_t *)));
    }
    struct cu_coroutine *coro = cu_reactor_get_current_coro(reactor);
    watchers = g_tree_lookup(reactor->fd_dict, &fd);
    g_assert(watchers);
    bool has = false;
    for (size_t i = 0; i < watchers->len; ++i) {
        if (g_array_index(watchers, cu_coroutine_t, i) == coro) {
            has = true;
            break;
        }
    }
    if (!has) {
        g_array_append_val(watchers, coro);
    }
    reactor->caller = coro->id;
    ++reactor->files_cnt;
    getcontext(&coro->context);
    if (reactor->caller == coro->id) {
        setcontext(&reactor->context);
    }
    --reactor->files_cnt;
    return read(fd, buffer, size);
}

ssize_t cu_write(int fd, void *buffer, size_t size, struct cu_reactor *reactor) {
    GArray *watchers = g_tree_lookup(reactor->fd_dict, &fd);
    if (watchers == NULL) {
        int *fdptr = malloc(sizeof(int));
        *fdptr = fd;
        g_tree_insert(reactor->fd_dict, fdptr, g_array_new(FALSE, FALSE, sizeof(cu_coroutine_t *)));
    }
    struct cu_coroutine *coro = cu_reactor_get_current_coro(reactor);
    watchers = g_tree_lookup(reactor->fd_dict, &fd);
    g_array_append_val(watchers, coro);
    reactor->caller = coro->id;
    ++reactor->files_cnt;
    getcontext(&coro->context);
    if (reactor->caller == coro->id) {
        setcontext(&reactor->context);
    }
    --reactor->files_cnt;
    return write(fd, buffer, size);
}