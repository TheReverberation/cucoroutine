#include "async_reactor.h"


#include <assert.h>
#include <stddef.h>
#include <string.h>


void 
async_reactor_init(
    async_reactor_t *reactor
) {
    memset(reactor->coros, 0, AS_REACTOR_MAX_COROS * sizeof(coroutine_t *));
    reactor->size = 0;
    reactor->caller = -1;
    reactor->current_coro = -1;
}


bool 
async_reactor_add_coro(
    async_reactor_t *reactor,
    coroutine_t *coro
) {
    if (reactor->size < AS_REACTOR_MAX_COROS) {
        reactor->coros[reactor->size++] = coro;
        return true;
    }
    return false;
}


coroutine_t *
async_reactor_get_current_coro(
    async_reactor_t *reactor
) {
    return reactor->current_coro != -1 ? reactor->coros[reactor->current_coro] : NULL;
}


void 
async_reactor_resume_coro(
    async_reactor_t *reactor
) {
    coroutine_t *coro = async_reactor_get_current_coro(reactor);
    if (coro->status == CORO_NOT_EXEC) {
        coro->status = CORO_PAUSE;
        coro->func();
    } else if (coro->status == CORO_PAUSE) {
        setcontext(&coro->context);
    } else if (coro->status == CORO_DONE) {
        setcontext(&reactor->backpoint);
    } else {
        assert(false);
    }
}


void 
async_reactor_run(
    async_reactor_t *reactor
) {
    bool has_undone_core = true;
    while (has_undone_core) {
        has_undone_core = false;
        for (size_t i = 0; i < reactor->size; ++i) {
            reactor->current_coro = i;
            reactor->caller = 0;
            getcontext(&(reactor->backpoint));
            if (reactor->caller == 0) {
                async_reactor_resume_coro(reactor);
            }
            if (async_reactor_get_current_coro(reactor) != NULL && 
                async_reactor_get_current_coro(reactor)->status != CORO_DONE
            ) {
                has_undone_core = true;
            }
        }
    }
}



void async_reactor_yield(
    async_reactor_t *reactor
) {
    coroutine_t *coro = async_reactor_get_current_coro(reactor);
    reactor->caller = coro->id;
    getcontext(&(coro->context));
    if (reactor->caller == coro->id) {
        setcontext(&(reactor->backpoint));
    }
}


void
async_reactor_coro_exit(
    async_reactor_t *reactor
) {
    coroutine_t *coro = async_reactor_get_current_coro(reactor);
    coro->status = CORO_DONE;
    //coro_delete(async_reactor_get_current_coro(reactor));
    reactor->caller = coro->id;
    setcontext(&(reactor->backpoint));
}