#include <stdlib.h>

#include <glib.h>

#include <reactor.h>

#include "coroutine_private.h"

static char const *coro_status_names[CORO_STATUS_END];

void coro_status_name_init__() {
    coro_status_names[CORO_NOT_EXEC] = "not exec";
    coro_status_names[CORO_RUNNING] = "running";
    coro_status_names[CORO_DONE] = "done";
}


char const *
coro_status_name(enum coro_status status) {
    return coro_status_names[status];
}



static int32_t id = 0;


cu_coroutine_t
cu_create(
    cu_func_t func,
    void *args,
    struct cu_reactor *reactor
) {
#ifdef CU_DEBUG
    g_assert(func);
    g_assert(reactor);
#endif
    struct cu_coroutine *coro = malloc(sizeof(struct cu_coroutine));
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

struct cu_coroutine *coros[1024];
int cpos = 0;

static void
coro_runner(
    struct cu_coroutine *coro
) {
    coro->func(coro->args);
    cu_exit(coro->reactor);
}

#define DEFAULT_STACK_SIZE (1L << 20)


cu_err_t
cu_coro_init(
    struct cu_coroutine *coro,
    cu_func_t func,
    void *args,
    struct cu_reactor *reactor
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
    // makecontext accept a 32bit arguments, see 'man makecontext'
    makecontext(&(coro->context), (void (*)(void))coro_runner, 2, coro);
    return CU_EOK;
}

void
coro_goto_begin__(
    struct cu_coroutine *coro
) {
    getcontext(&(coro->context));
    coro->context.uc_stack.ss_sp = coro->stack;
    coro->context.uc_stack.ss_size = DEFAULT_STACK_SIZE;
    coro->context.uc_link = NULL;
    makecontext(&(coro->context), (void (*)(void))coro_runner, 2, coro);
}

void 
back_to_coro__(
    struct cu_coroutine *coro
) {
    setcontext(&(coro->context));
}

void
cu_coro_destroy(
    struct cu_coroutine *coro
) {
#ifdef CU_DEBUG
    if (coro->status != CORO_DONE) {
        g_warning("Coro [id = %d] is deleted but not done", coro->id);
    }
#endif
    free(coro->stack);
    coro->stack = NULL;
}
