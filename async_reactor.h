#pragma once


#include "coroutine.h"


#include <glib.h>

#include <ucontext.h>

#include <stdint.h>
#include <stdbool.h>



#define AS_REACTOR_MAX_COROS 4096LL

typedef struct async_reactor {  
    uint32_t size;
    coroutine_t *current_coro;
    int caller;
    ucontext_t context;
    GTree *schedule; // tree of pair<guint64, coroutine_t>
} async_reactor_t;


void 
async_reactor_init(
    async_reactor_t *reactor
);

coroutine_t *
async_reactor_add_coro(
    async_reactor_t *reactor,
    coro_func_t func,
    void *args
);

void async_reactor_yield(
    async_reactor_t *reactor,
    guint64 run_after_u
);

void 
async_reactor_resume_coro(
    async_reactor_t *reactor
);

void 
async_reactor_run(
    async_reactor_t *reactor
);

coroutine_t *
async_reactor_get_current_coro(
    async_reactor_t *reactor
);

void
async_reactor_coro_exit(
    async_reactor_t *reactor
);