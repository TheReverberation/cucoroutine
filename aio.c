#include "aio.h"

#include "coro_status.h"

async_reactor_t default_reactor;


void 
aio_init() {
    coro_status_name_init();
    async_reactor_init(&default_reactor);
}


coroutine_t *
aio_add_coro(
    coro_func_t func,
    void *args
) {
    return async_reactor_add_coro(&default_reactor, func, args);
}


void
aio_run() {
    async_reactor_run(&default_reactor);
}


void 
aio_coro_exit() {
    async_reactor_coro_exit(&default_reactor);
}



void 
aio_coro_yield(guint64 run_after_u) {
    async_reactor_yield(&default_reactor, run_after_u);
}