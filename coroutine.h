#pragma once

#include "corodefs.h"
#include "coro_status.h"

#include <ucontext.h>

typedef struct coroutine {
    coro_func_t func;
    enum coro_status status;
    ucontext_t context;
    int id;
} coroutine_t;

coroutine_t *
coro_make(coro_func_t func);

void 
coro_delete(coroutine_t *coro);
