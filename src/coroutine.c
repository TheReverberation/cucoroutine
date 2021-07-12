#include <stdlib.h>

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
#ifdef CU_DEBUG
    g_assert(func);
    g_assert(reactor);
#endif
    cu_coroutine_t *coro = malloc(sizeof(cu_coroutine_t));
#ifdef CU_DEBUG
    g_assert(coro);
#endif
    if (coro == NULL) {
        cu_seterr(errno);
        return NULL;
    }
    cu_err_t err;
    if ((err = cu_coro_init(coro, func, args, reactor)) != CU_EOK) {
        cu_seterr(err);
        return NULL;
    }
    return coro;
}
cu_coroutine_t *coros[1024];
int cpos = 0;

static void
coro_runner(
    int coropos
) {
    cu_coroutine_t *coro = coros[coropos];
    coro->func(coro->args);
    cu_coro_exit(coro->reactor);
}

#define DEFAULT_STACK_SIZE (1L << 20)


cu_err_t
cu_coro_init(
    cu_coroutine_t *coro,
    cu_func_t func,
    void *args,
    cu_reactor_t *reactor
) {
#ifdef CU_DEBUG
    g_assert(coro);
    g_assert(func);
    g_assert(reactor);
#endif
    coro->func = func;
    coro->args = args;
    coro->status = CORO_NOT_EXEC;
    coro->reactor = reactor;
    coro->id = ++id;
    // todo: user stack config
    coro->stack = malloc(DEFAULT_STACK_SIZE);
    if (coro->stack == NULL) {
        return errno;
    }
    getcontext(&(coro->context));
    coro->context.uc_stack.ss_sp = coro->stack;
    coro->context.uc_stack.ss_size = DEFAULT_STACK_SIZE;
    coro->context.uc_link = NULL;
    coros[++cpos] = coro;
    makecontext(&(coro->context), (void (*)(void))coro_runner, 1, cpos);
    return CU_EOK;
}

void
coro_goto_begin__(
    cu_coroutine_t *coro
) {
    getcontext(&(coro->context));
    coro->context.uc_stack.ss_sp = coro->stack;
    coro->context.uc_stack.ss_size = DEFAULT_STACK_SIZE;
    coro->context.uc_link = NULL;
    makecontext(&(coro->context), (void (*)(void))coro_runner, 1, coro);
}

void 
back_to_coro__(
    cu_coroutine_t *coro
) {
    setcontext(&(coro->context));
}

void
cu_coro_destroy(
    cu_coroutine_t *coro
) {
#ifdef CU_DEBUG
    if (coro->status != CORO_DONE) {
        g_warning("Coro [id = %d] is deleted but not done", coro->id);
    }
#endif
    free(coro->stack);
    coro->stack = NULL;
}
