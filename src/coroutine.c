#include <stdlib.h>
#include <assert.h>

#include <glib.h>

#include <coroutine.h>
#include <async_reactor.h>



static int32_t id = 0;

coroutine_t *
coro_make(
    coro_func_t func,
    void *args,
    async_reactor_t *reactor
) {
    g_assert(func);
    coroutine_t *coro = malloc(sizeof(coroutine_t));
    g_assert(coro);
    coro_init(coro, func, args, reactor);
    return coro;
}

static void
coro_runner(
    async_reactor_t *reactor,
    coroutine_t *coro
) {
    coro->func(coro->args);
    async_reactor_coro_exit(reactor);
}

aio_err_t
coro_init(
    coroutine_t *coro,
    coro_func_t func,
    void *args,
    async_reactor_t *reactor
) {
    assert(coro);
    assert(func);
    coro->func = func;
    coro->args = args;
    coro->status = CORO_NOT_EXEC;
    coro->id = ++id;
    coro->stack = malloc(1 << 16);
    assert(coro->stack);
    getcontext(&(coro->context));
    coro->context.uc_stack.ss_sp = coro->stack;
    coro->context.uc_stack.ss_size = 1 << 16;
    coro->context.uc_link = NULL;
    makecontext(&(coro->context), (void (*)(void))coro_runner, 2, reactor, coro);
    return coro;
}

void
_coro_goto_begin(
    coroutine_t *coro,
    async_reactor_t *reactor
) {
    getcontext(&(coro->context));
    coro->context.uc_stack.ss_sp = coro->stack;
    coro->context.uc_stack.ss_size = 1 << 16;
    coro->context.uc_link = NULL;
    makecontext(&(coro->context), (void (*)(void))coro_runner, 2, reactor, coro);
}

void 
_back_to_coro(
    coroutine_t *coro
) {
    setcontext(&(coro->context));
}

void
coro_destroy(
    coroutine_t *coro
) {
    if (coro->status != CORO_DONE) {
        g_warning("Coro [id = %d] is deleted but not done", coro->id);
    }
    free(coro->stack);
    coro->stack = NULL;
}
