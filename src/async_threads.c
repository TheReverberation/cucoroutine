#include <stdlib.h>

#include <sys/types.h>

#include <glib.h>
#include <stdio.h>
#include "async_threads.h"


static GTree *thread_metadata;

struct thread_metadata {
    cu_coroutine_t *coro;
    ucontext_t exit_context;
};

static int
sign(int64_t x) {
    if (x < 0) {
        return -1;
    } else if (x > 0) {
        return 1;
    } else {
        return 0;
    }
}

static gint
pid_compare(
    gconstpointer _a,
    gconstpointer _b,
    gpointer _data
) {
    return sign((int64_t)*(pthread_t *)_a - (int64_t)*(pthread_t *)_b);
}

void
cu_threads_init__() {
    thread_metadata = g_tree_new_full(pid_compare, NULL, free, free);
}

void cu_threads_destroy__() {
    g_tree_destroy(thread_metadata);
}

static void 
setmeta(
    pthread_t thr, 
    void *data
) {
    pthread_t *ptr = malloc(sizeof(pthread_t));
    *ptr = thr;
    g_tree_insert(thread_metadata, ptr, data);
}

static void *
getmeta(pthread_t thr) {
    return g_tree_lookup(thread_metadata, &thr);
}

static void *
compute(void *arg) {
    cu_coroutine_t *coro = arg;
    volatile bool to_exit = false;

    struct thread_metadata *meta = malloc(sizeof(struct thread_metadata));
    meta->coro = coro;
    getcontext(&meta->exit_context);
    setmeta(pthread_self(), meta);
    if (!to_exit) {
        to_exit = true;
        setcontext(&coro->context);
    }
    return NULL;
}



void 
cu_begin_compute(cu_reactor_t *reactor) {
    cu_coroutine_t *coro = cu_reactor_get_current_coro(reactor);
    coro->status = CORO_RUNNUNG_IN_THREAD;
    reactor->caller = coro->id;
    ++reactor->threads;
    volatile bool to_reactor = true;
    getcontext(&coro->context);
    if (to_reactor) {
        to_reactor = false;
        pthread_t thr;
        pthread_create(&thr, NULL, compute, coro);
        setcontext(&reactor->context);
    }
}


void 
cu_end_compute(cu_reactor_t *reactor) {
    struct thread_metadata *meta = getmeta(pthread_self());
    cu_coroutine_t *thread_coro = meta->coro;

    getcontext(&thread_coro->context);
    if (thread_coro->status == CORO_RUNNUNG_IN_THREAD) {
        thread_coro->status = CORO_RUNNING;
        pthread_mutex_lock(&reactor->mutex);
        cu_reactor_add_coro(reactor, thread_coro);
        --reactor->threads;
        pthread_cond_signal(&reactor->thread_exit);
        pthread_mutex_unlock(&reactor->mutex);
        setcontext(&meta->exit_context);
    }
}
