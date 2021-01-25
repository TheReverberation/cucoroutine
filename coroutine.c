#include "coroutine.h"

#include <stdlib.h>
#include <assert.h>

static int id = 0;

coroutine_t *
coro_make(
    coro_func_t func
) {
    coroutine_t *coro = malloc(sizeof(coroutine_t));
    coro->func = func;
    coro->status = CORO_NOT_EXEC;
    coro->id = ++id;
    void *stack = malloc(1 << 16);
    assert(stack);
    coro->stack = stack + (1 << 16) - 1;
    return coro;
}



void 
coro_delete(coroutine_t *coro) {
    //free(coro->context.stack);
    //free(coro);
}