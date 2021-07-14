#pragma once

#include <ucontext.h>

#include "coroutine.h"

enum coro_status {
    CORO_NOT_EXEC,
    CORO_RUNNING,
    CORO_RUNNUNG_IN_THREAD,
    CORO_DONE,
    CORO_STATUS_END
};


void
coro_status_name_init__();

char const *
coro_status_name(enum coro_status status);

typedef struct cu_coroutine {
    cu_func_t func;
    void *args;
    struct cu_reactor *reactor;
    enum coro_status status;
    ucontext_t context;
    void *stack;
    int32_t id;
} *cu_coroutine_t;


void
coro_goto_begin__(
    struct cu_coroutine *coro
);

void
back_to_coro__(
    struct cu_coroutine *coro
);