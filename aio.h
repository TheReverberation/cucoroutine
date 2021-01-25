#pragma once

#include "async_reactor.h"
#include "coroutine.h"

#include <stdbool.h>

extern async_reactor_t default_reactor;

void 
aio_init();

bool
aio_add_coro(
    coroutine_t *coro
);

void
aio_run();

void 
aio_coro_exit();

/* #define aio_coro_yield() if (setjmp(async_reactor_get_current_coro(&default_reactor)->context.backpoint) == 0) { \
        longjmp(default_reactor.backpoint, 1);            \
    } \ */

void 
aio_coro_yield();