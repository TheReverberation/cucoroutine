#include <stdlib.h>
#include <assert.h>

#include <glib.h>

#include <reactor.h>
#include <coroutine.h>



static int32_t id = 0;

cu_coroutine_t *
cu_make(
    cu_func_t func,
    void *args,
    cu_reactor_t *reactor
) {
    g_assert(func);
    cu_coroutine_t *coro = malloc(sizeof(cu_coroutine_t));
    g_assert(coro);
    cu_coro_init(coro, func, args, reactor);
    return coro;
}

static void
coro_runner(
    cu_reactor_t *reactor,
    cu_coroutine_t *coro
) {
    coro->func(coro->args);
    cu_reactor_coro_exit(reactor);
}

cu_err_t
cu_coro_init(
    cu_coroutine_t *coro,
    cu_func_t func,
    void *args,
    cu_reactor_t *reactor
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
    cu_coroutine_t *coro,
    cu_reactor_t *reactor
) {
    getcontext(&(coro->context));
    coro->context.uc_stack.ss_sp = coro->stack;
    coro->context.uc_stack.ss_size = 1 << 16;
    coro->context.uc_link = NULL;
    makecontext(&(coro->context), (void (*)(void))coro_runner, 2, reactor, coro);
}

void 
_back_to_coro(
    cu_coroutine_t *coro
) {
    setcontext(&(coro->context));
}

void
cu_coro_destroy(
    cu_coroutine_t *coro
) {
    if (coro->status != CORO_DONE) {
        g_warning("Coro [id = %d] is deleted but not done", coro->id);
    }
    free(coro->stack);
    coro->stack = NULL;
}
