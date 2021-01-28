#include "async_reactor.h"


#include <assert.h>
#include <time.h>
#include <stddef.h>
#include <string.h>


#include <unistd.h>

static gint
clock_cmp(
    gconstpointer a,
    gconstpointer b
) {
    if (*(guint64 *)a < *(guint64 *)b) {
        return -1;
    } else if (*(guint64 *)a > *(guint64 *)b) {
        return 1;
    } else {
        return 0;
    }
}

void 
async_reactor_init(
    async_reactor_t *reactor
) {
    reactor->size = 0;
    reactor->caller = -1;
    reactor->current_coro = NULL;
    reactor->schedule = g_tree_new(clock_cmp);
}




coroutine_t *
async_reactor_add_coro(
    async_reactor_t *reactor,
    coro_func_t func,
    void *args
) {
    guint64 *now = malloc(sizeof(guint64));
    *now = g_get_monotonic_time();
    coroutine_t *coro = coro_make(func, args);
    g_tree_insert(reactor->schedule, now, coro);
    return coro;
}


coroutine_t *
async_reactor_get_current_coro(
    async_reactor_t *reactor
) {
    return reactor->current_coro;
}


void 
async_reactor_resume_coro(
    async_reactor_t *reactor
) {
    coroutine_t *coro = async_reactor_get_current_coro(reactor);
    if (coro->status == CORO_NOT_EXEC) {
        coro->status = CORO_PAUSE;
        getcontext(&(coro->context));
        coro->context.uc_stack.ss_sp = coro->stack;
        coro->context.uc_stack.ss_size = 1 << 16;
        coro->context.uc_link = NULL;
        makecontext(&(coro->context), (void (*)(void))coro->func, 1, coro->args);
        setcontext(&(coro->context));
    } else if (coro->status == CORO_PAUSE) {
        setcontext(&coro->context);
    } else if (coro->status == CORO_DONE) {
        reactor->caller = coro->id;
        setcontext(&reactor->context);
    } else {
        assert(false);
    }
}

struct schedule_pair {
    guint64 *run_time;
    coroutine_t *coro;
};

static gboolean
stop_on_first(
    gpointer run_time,
    gpointer coro,
    gpointer user_data
) {
    ((struct schedule_pair *)user_data)->run_time = run_time;
    ((struct schedule_pair *)user_data)->coro = coro;
    return TRUE;
}



static void
g_tree_first(
    GTree *tree,
    struct schedule_pair *out
) {
    g_tree_foreach(tree, stop_on_first, out);
}

void 
async_reactor_run(
    async_reactor_t *reactor
) {
    while (g_tree_nnodes(reactor->schedule) > 0) {
        struct schedule_pair first;
        g_tree_first(reactor->schedule, &first);

        guint64 const run_time = *(first.run_time);
        coroutine_t *coro = first.coro;
        g_tree_remove(reactor->schedule, &run_time);

        guint64 const now = g_get_monotonic_time();
        if (now < run_time) {
            g_usleep((run_time - now) * 1000);
        }
        reactor->current_coro = coro;
        reactor->caller = 0;
        getcontext(&(reactor->context));
        if (reactor->caller == 0) {
            async_reactor_resume_coro(reactor);
        }
    }
}



void async_reactor_yield(
    async_reactor_t *reactor,
    guint64 run_after_u
) {
    coroutine_t *coro = async_reactor_get_current_coro(reactor);
    guint64 *run_time = malloc(sizeof(guint64));
    *run_time = g_get_monotonic_time() + run_after_u;
    g_tree_insert(reactor->schedule, run_time, coro);
    reactor->caller = coro->id;
    getcontext(&(coro->context));
    if (reactor->caller == coro->id) {
        setcontext(&(reactor->context));
    }
}


void
async_reactor_coro_exit(
    async_reactor_t *reactor
) {
    coroutine_t *coro = async_reactor_get_current_coro(reactor);
    coro->status = CORO_DONE;
    reactor->caller = coro->id;
    setcontext(&(reactor->context));
}