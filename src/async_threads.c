#include <sys/types.h>

#include <glib.h>

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
async_threads_init() {
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
    coroutine_t *coro = arg;
    setmeta(pthread_self(), coro);
    setcontext(&coro->context);
}

void 
begin_compute(async_reactor_t *reactor) {
    coroutine_t *coro = async_reactor_get_current_coro(reactor);
    coro->status = CORO_RUNNUNG_IN_THREAD;
    reactor->caller = coro->id;
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
end_compute(async_reactor_t *reactor) {
    coroutine_t *thread_coro = getmeta(pthread_self());
    getcontext(&thread_coro->context);
    if (thread_coro->status == CORO_RUNNUNG_IN_THREAD) {
        thread_coro->status = CORO_RUNNING;
        pthread_mutex_lock(&reactor->mutex);
        async_reactor_add_coro(reactor, thread_coro);
        pthread_mutex_unlock(&reactor->mutex);
        pthread_exit(NULL);
    }
}
