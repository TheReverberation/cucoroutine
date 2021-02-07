#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <ucontext.h>

#include <glib.h>

#include "coroutine.h"

#define MS * 1000

typedef struct async_reactor {  
    GArray *maked_coros; // coroutine_t * array
    coroutine_t *current_coro;
    int caller;
    ucontext_t context;
    GTree *schedule; // tree of pair<guint64[2] = {runtime, coro.id}, coroutine_t>
} async_reactor_t;


void 
async_reactor_init(
    async_reactor_t *reactor
);

void
async_reactor_make_coro(
    async_reactor_t *reactor,
    coro_func_t func,
    void *args
);

void
async_reactor_add_coro(
    async_reactor_t *reactor,
    coroutine_t *coro
);

void async_reactor_yield_at_time(
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


