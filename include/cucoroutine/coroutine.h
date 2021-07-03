#pragma once

#include <errno.h>
#include <stdbool.h>
#include <ucontext.h>

#include "coro_status.h"
#include "errors.h"

typedef struct cu_reactor cu_reactor_t;

typedef void (*cu_func_t)(void *);

typedef struct cu_coroutine {
    cu_func_t func;
    void *args;
    cu_reactor_t *reactor;
    enum coro_status status;
    ucontext_t context;
    void *stack;
    int32_t id;
} cu_coroutine_t;

cu_coroutine_t *
cu_make(
    cu_func_t func,
    void *args,
    cu_reactor_t *reactor
);

cu_err_t
cu_coro_init(
    cu_coroutine_t *coro,
    cu_func_t func,
    void *args,
    cu_reactor_t *reactor
);

void
cu_coro_destroy(cu_coroutine_t *coro);

void
coro_goto_begin__(
    cu_coroutine_t *coro
);

void 
back_to_coro__(
    cu_coroutine_t *coro
);

