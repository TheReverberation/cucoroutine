#include <stdlib.h>

#include <sys/types.h>

#include <glib.h>
#include <stdio.h>
#include "async_threads.h"


static GTree *thread_metadata;

static gint
pid_compare(
    gconstpointer _a,
    gconstpointer _b,
    gpointer _data
) {
    return *(pthread_t *)_a - *(pthread_t *)_b;
}

void
cu_threads_init__() {
    thread_metadata = g_tree_new_full(pid_compare, NULL, free, free);
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
    setmeta(pthread_self(), coro);
    setcontext(&coro->context);
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
    printf("END BEGIN COMPUTE\n");
}


void 
cu_end_compute(cu_reactor_t *reactor) {
    cu_coroutine_t *thread_coro = getmeta(pthread_self());
    printf("end compute\n");
    getcontext(&thread_coro->context);
    if (thread_coro->status == CORO_RUNNUNG_IN_THREAD) {
        thread_coro->status = CORO_RUNNING;
        pthread_mutex_lock(&reactor->mutex);
        cu_reactor_add_coro(reactor, thread_coro);
        --reactor->threads;
        pthread_cond_signal(&reactor->thread_exit);
        pthread_mutex_unlock(&reactor->mutex);
        printf("THREAD EXIT\n");
        pthread_exit(NULL);
    }
    printf("END COMPUTE\n");
}
