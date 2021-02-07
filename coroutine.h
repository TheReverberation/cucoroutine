#pragma once

#include <stdbool.h>

#include <ucontext.h>

#include "coro_status.h"

typedef struct async_reactor async_reactor_t;

typedef void (*coro_func_t)(void *);


typedef struct coroutine {
    coro_func_t func;
    void *args;
    enum coro_status status;
    ucontext_t context;
    void *stack;
    int32_t id;
} coroutine_t;

coroutine_t *
coro_make(
    coro_func_t func,
    void *args,
    async_reactor_t *reactor
);

bool
coro_init(
    coroutine_t *coro,
    coro_func_t func,
    void *args,
    async_reactor_t *reactor
);

void
_coro_goto_begin(
    coroutine_t *coro,
    async_reactor_t *reactor
);

void 
back_to_coro(
    coroutine_t *coro
);

void 
coro_destroy(coroutine_t *coro);
