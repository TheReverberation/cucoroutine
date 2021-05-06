#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <unistd.h>

#include "reactor.h"

static gint
run_time_cmp(
    gconstpointer _a,
    gconstpointer _b,
    gpointer _data
) {
    uint64_t const *a = _a, *b = _b;
    if (a[0] < b[0]) {
        return -1;
    } else if (a[0] > b[0]) {
        return 1;
    } else {
        if (a[1] < b[1]) {
            return -1;
        } else if (a[1] > b[1]) {
            return 1;
        } else {
            return 0;
        }
    }
}




cu_err_t 
cu_reactor_init(
    cu_reactor_t *reactor
) {
    reactor->caller = -1;
    reactor->current_coro = NULL;
    reactor->maked_coros = g_array_new(FALSE, FALSE, sizeof(cu_coroutine_t *));
    reactor->schedule = g_tree_new_full(run_time_cmp, NULL, free, NULL);
}

void
cu_reactor_make_coro(
    cu_reactor_t *reactor,
    cu_func_t func,
    void *args
) {
    cu_coroutine_t *coro = cu_make(func, args, reactor);
    g_array_append_val(reactor->maked_coros, coro);
    cu_reactor_add_coro(reactor, coro);
}

void
cu_reactor_add_coro(
    cu_reactor_t *reactor,
    cu_coroutine_t *coro
) {
    guint64 *now = malloc(2 * sizeof(uint64_t));
    now[0] = g_get_monotonic_time();
    now[1] = coro->id;
    g_tree_insert(reactor->schedule, now, coro);
}

cu_coroutine_t *
cu_reactor_get_current_coro(
    cu_reactor_t *reactor
) {
    return reactor->current_coro;
}


void 
cu_reactor_resume_coro(
    cu_reactor_t *reactor
) {
    cu_coroutine_t *coro = cu_reactor_get_current_coro(reactor);
    if (coro->status == CORO_NOT_EXEC) {
        coro->status = CORO_RUNNING;
        _back_to_coro(coro);
    } else if (coro->status == CORO_RUNNING) {
        _back_to_coro(coro);
    } else if (coro->status == CORO_DONE) {
        reactor->caller = coro->id;
        return;
    } else {
        assert(false);
    }
}

struct schedule_pair {
    guint64 const *run_time;
    cu_coroutine_t *coro;
};

static gboolean
stop_at_first(
    gpointer run_time,
    gpointer coro,
    gpointer user_data
) {
    ((struct schedule_pair *)user_data)->run_time = run_time;
    ((struct schedule_pair *)user_data)->coro = coro;
    return TRUE;
}

// static gboolean
// print_node(
//     gpointer run_time,
//     gpointer _coro,
//     gpointer user_data
// ) {
//     cu_coroutine_t *coro = _coro;
//     printf("[id = %d]\n", coro->id);
//     return FALSE;
// }

// static void
// g_tree_print_all(
//     GTree *tree
// ) {
//     printf("schedule\n");
//     g_tree_foreach(tree, print_node, NULL);
//     printf("--------\n");
// }

static void
g_tree_first(
    GTree *tree,
    struct schedule_pair *out
) {
    g_tree_foreach(tree, stop_at_first, out);
}

static void
cu_reactor_destroy(
    cu_reactor_t *reactor
) {
    for (size_t i = 0; i < reactor->maked_coros->len; ++i) {
        cu_coroutine_t *coro = g_array_index(reactor->maked_coros, cu_coroutine_t *, i);
        //printf("coro[id = %d] destroyed\n", coro->id);
        cu_coro_destroy(coro);
        free(coro);
    }
    g_array_free(reactor->maked_coros, TRUE);
    g_tree_destroy(reactor->schedule);
}

cu_err_t 
cu_reactor_run(
    cu_reactor_t *reactor
) {
    while (g_tree_nnodes(reactor->schedule) > 0) {
        //g_tree_print_all(reactor->schedule);
        struct schedule_pair first;
        g_tree_first(reactor->schedule, &first);

        guint64 const run_time = *(first.run_time);
        cu_coroutine_t *coro = first.coro;
        //printf("%s coro: %d\n", __func__, coro->id);
        g_tree_remove(reactor->schedule, first.run_time);
        guint64 const now = g_get_monotonic_time();
        if (now < run_time) {
            g_usleep(run_time - now);
        }
        reactor->current_coro = coro;
        reactor->caller = 0;
        getcontext(&(reactor->context));
        if (reactor->caller == 0) {
            cu_reactor_resume_coro(reactor);
        }
    }
    cu_reactor_destroy(reactor);
}



void cu_reactor_yield_at_time(
    cu_reactor_t *reactor,
    guint64 run_after_u
) {
    cu_coroutine_t *coro = cu_reactor_get_current_coro(reactor);
    guint64 *run_time = malloc(2 * sizeof(uint64_t));
    run_time[0] = g_get_monotonic_time() + run_after_u;
    run_time[1] = coro->id;
    g_tree_insert(reactor->schedule, run_time, coro);
    reactor->caller = coro->id;
    getcontext(&(coro->context));
    if (reactor->caller == coro->id) {
        setcontext(&(reactor->context));
    }
}


void
cu_reactor_coro_exit(
    cu_reactor_t *reactor
) {
    cu_coroutine_t *coro = cu_reactor_get_current_coro(reactor);
    coro->status = CORO_DONE;
    reactor->caller = coro->id;
    setcontext(&(reactor->context));
}
