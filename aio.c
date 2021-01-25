#include "aio.h"

#include "coro_status.h"

async_reactor_t default_reactor;


void 
aio_init() {
    coro_status_name_init();
    async_reactor_init(&default_reactor);
}


bool
aio_add_coro(
    coroutine_t *coro
) {
    return async_reactor_add_coro(&default_reactor, coro);
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
aio_coro_yield() {
    async_reactor_yield(&default_reactor);
}