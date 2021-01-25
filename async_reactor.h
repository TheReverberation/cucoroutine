#pragma once


#include "coroutine.h"

#include <ucontext.h>

#include <stdint.h>
#include <stdbool.h>



#define AS_REACTOR_MAX_COROS 1024

typedef struct async_reactor {  
    coroutine_t *coros[AS_REACTOR_MAX_COROS];
    uint32_t size;
    int current_coro;
    int caller;
    ucontext_t backpoint;
} async_reactor_t;


void 
async_reactor_init(
    async_reactor_t *reactor
);

bool 
async_reactor_add_coro(
    async_reactor_t *reactor,
    coroutine_t *coro
);

void async_reactor_yield(
    async_reactor_t *reactor
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