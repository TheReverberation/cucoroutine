#pragma once

#include "errors.h"


typedef struct cu_reactor *cu_reactor_t;

typedef void (*cu_func_t)(void *);
typedef struct cu_coroutine *cu_coroutine_t;


cu_coroutine_t
cu_make(
    cu_func_t func,
    void *args,
    cu_reactor_t reactor
);

cu_err_t
cu_coro_init(
    cu_coroutine_t coro,
    cu_func_t func,
    void *args,
    cu_reactor_t reactor
);

void
cu_coro_destroy(cu_coroutine_t coro);



