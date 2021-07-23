#include <stddef.h>
#include <stdio.h>

#include "coroutine_private.h"
#include "reactor.h"
#include "reactor_private.h"

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

static gint
int_compare(
    gconstpointer a_,
    gconstpointer b_,
    gpointer data_
) {
    int a = *(int *)a_, b = *(int *)b_;
    return a - b;
}

static void
coroutine_array_destroy(gpointer array_) {
    GArray *array = array_;
    g_array_free(array, TRUE);
}

#define CU_MAX_FILES 1024

cu_err_t 
cu_reactor_init(
    struct cu_reactor **reactor
) {
#ifdef CU_DEBUG
    g_assert(reactor);
#endif
    *reactor = malloc(sizeof(struct cu_reactor));
    (*reactor)->files_cnt = 0;
    (*reactor)->caller = -1;
    (*reactor)->current_coro = NULL;
    (*reactor)->threads = 0;
    (*reactor)->coroutines = g_array_new(FALSE, FALSE, sizeof(cu_coroutine_t));
    (*reactor)->files = g_array_new(FALSE, FALSE, sizeof(struct epoll_event *));
    g_array_set_clear_func((*reactor)->files, free);
    (*reactor)->schedule = g_tree_new_full(run_time_cmp, NULL, free, NULL);
    (*reactor)->fd_dict = g_tree_new_full(int_compare, NULL, free, coroutine_array_destroy);
    int rcode = pthread_mutex_init(&((*reactor)->mutex), NULL);
    if (rcode != 0) {
        goto SCHEDULE_CLEANUP;
    }
    rcode = pthread_cond_init(&((*reactor)->thread_exit), NULL);
    if (rcode != 0) {
        goto MUTEX_CLEANUP;
    }

    rcode = (*reactor)->epollfd = epoll_create(CU_MAX_FILES);
    if (rcode == -1) {
        goto THREAD_EXIT_CLEANUP;
    }
    return CU_EOK;
THREAD_EXIT_CLEANUP:
    pthread_cond_destroy(&(*reactor)->thread_exit);
MUTEX_CLEANUP:
    pthread_mutex_destroy(&(*reactor)->mutex);
SCHEDULE_CLEANUP:
    g_tree_destroy((*reactor)->fd_dict);
    g_tree_destroy((*reactor)->schedule);
    g_array_free((*reactor)->files, TRUE);
    g_array_free((*reactor)->coroutines, TRUE);
    return errno;
}

void
cu_reactor_create_coro(
    struct cu_reactor *reactor,
    cu_func_t func,
    void *args
) {
    cu_coroutine_t coro = cu_create(func, args, reactor);
    g_array_append_val(reactor->coroutines, coro);
    cu_reactor_add(reactor, coro);
}

void
cu_reactor_add(
    struct cu_reactor *reactor,
    cu_coroutine_t coro
) {
#ifdef CU_DEBUG
    g_assert(reactor);
    g_assert(coro);
#endif
    guint64 *now = malloc(2 * sizeof(guint64));
    now[0] = g_get_monotonic_time();
    now[1] = coro->id;
    g_tree_insert(reactor->schedule, now, coro);
}

cu_coroutine_t
cu_self(
    struct cu_reactor *reactor
) {
#ifdef CU_DEBUG
    g_assert(reactor);
#endif
    return reactor->current_coro;
}


void 
cu_reactor_resume_coro(
    struct cu_reactor *reactor
) {
#ifdef CU_DEBUG
    g_assert(reactor);
#endif
    struct cu_coroutine *coro = cu_self(reactor);
    if (coro->status == CORO_NOT_EXEC) {
        coro->status = CORO_RUNNING;
        back_to_coro__(coro);
    } else if (coro->status == CORO_RUNNING) {
        back_to_coro__(coro);
    } else if (coro->status == CORO_DONE) {
        reactor->caller = coro->id;
        return;
    } else {
        g_error("Coro has undefined status");
    }
}

struct schedule_pair {
    guint64 const *run_time;
    struct cu_coroutine *coro;
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


static void
g_tree_first(
    GTree *tree,
    struct schedule_pair *out
) {
    g_tree_foreach(tree, stop_at_first, out);
}

static void
cu_reactor_destroy(
    struct cu_reactor *reactor
) {
#ifdef CU_DEBUG
    g_assert(reactor);
#endif
    for (size_t i = 0; i < reactor->coroutines->len; ++i) {
        struct cu_coroutine *coro = g_array_index(reactor->coroutines, cu_coroutine_t, i);
        cu_coro_destroy(coro);
        free(coro);
    }
    g_array_free(reactor->coroutines, TRUE);
    g_tree_destroy(reactor->fd_dict);
    g_tree_destroy(reactor->schedule);
    g_array_free(reactor->files, TRUE);
}

static void
serve_epoll(struct cu_reactor *reactor, int timeout) {
    struct epoll_event events[CU_MAX_FILES];

    int nfds = epoll_wait(reactor->epollfd, events, CU_MAX_FILES, timeout);
    for (int i = 0; i < nfds; ++i) {
        int fd = events[i].data.fd;
        GArray *fd_watchers = g_tree_lookup(reactor->fd_dict, &fd);
        g_assert(fd_watchers);
        for (int j = 0; j < fd_watchers->len; ++j) {
            cu_reactor_add(reactor, g_array_index(fd_watchers, cu_coroutine_t, j));
        }
        g_array_set_size(fd_watchers, 0);
    }
}

//static gboolean
// print_node(
//     gpointer run_time,
//     gpointer _coro,
//     gpointer user_data
// ) {
//     cu_coroutine_t coro = _coro;
//     printf("[id = %d]\n", coro->id);
//     return FALSE;
// }
//
// static void
// g_tree_print_all(
//     GTree *tree
// ) {
//     printf("schedule\n");
//     g_tree_foreach(tree, print_node, NULL);
//     printf("--------\n");
// }


static void
serve_schedule(struct cu_reactor *reactor) {
    struct schedule_pair first;
    g_tree_first(reactor->schedule, &first);

    guint64 const run_time = *(first.run_time);
    cu_coroutine_t coro = first.coro;

    guint64 now = g_get_monotonic_time();
    if (now < run_time) {
        serve_epoll(reactor, (int) (run_time - now) / 1000);
    }
    now = g_get_monotonic_time();
    if (now < run_time) {
        g_usleep(run_time - now);
    }
    g_tree_remove(reactor->schedule, first.run_time);
    reactor->current_coro = coro;
    reactor->caller = 0;
    getcontext(&(reactor->context));
    if (reactor->caller == 0) {
        cu_reactor_resume_coro(reactor);
    }
}

// wait until someone thread exit
static void
serve_threads(struct cu_reactor *reactor) {
    int last_threads = reactor->threads;
    while (reactor->threads == last_threads) {
        pthread_cond_wait(&reactor->thread_exit, &reactor->mutex);
    }
}

cu_err_t 
cu_reactor_run(
    struct cu_reactor *reactor
) {
#ifdef CU_DEBUG
    g_assert(reactor);
#endif

    while (g_tree_nnodes(reactor->schedule) > 0 || reactor->threads > 0 || reactor->files_cnt > 0) {
        if (g_tree_nnodes(reactor->schedule) > 0) {
            serve_schedule(reactor);
        } else if (reactor->files_cnt > 0) {
            serve_epoll(reactor, -1);
        } else if (reactor->threads > 0) {
            serve_threads(reactor);
        }
        pthread_mutex_unlock(&reactor->mutex);
        g_usleep(0);
        pthread_mutex_lock(&reactor->mutex);
    }
    cu_reactor_destroy(reactor);
    return CU_EOK;
}


void cu_yield_at_time(
    int64_t run_after_u,
    struct cu_reactor *reactor
) {
#ifdef CU_DEBUG
    g_assert(reactor);
    g_assert(run_after_u >= 0);
#endif
    struct cu_coroutine *coro = cu_self(reactor);
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
cu_exit(
    struct cu_reactor *reactor
) {
#ifdef CU_DEBUG
    g_assert(reactor);
#endif
    struct cu_coroutine *coro = cu_self(reactor);
    coro->status = CORO_DONE;
    reactor->caller = coro->id;
    setcontext(&(reactor->context));
}
