#include "thread.h"

#include <glib.h>


static void *
run(void *thread_) {
    cu_thread_t *thread = thread_;
    thread->func(thread->arg);
    if (thread->coro != NULL) {
        pthread_mutex_lock(&thread->coro->reactor->mutex);
        cu_reactor_add_coro(thread->coro->reactor, thread->coro);
        --thread->coro->reactor->threads;
        pthread_cond_signal(&thread->coro->reactor->thread_exit);
        pthread_mutex_unlock(&thread->coro->reactor->mutex);
    }
    return NULL;
}

cu_err_t cu_thread_create(cu_thread_t *thr, void (*func)(void *), void *arg) {
    thr->func = func;
    thr->arg = arg;
    thr->coro = NULL;
    pthread_t pthr;
    pthread_create(&pthr, NULL, run, thr);
}

cu_err_t cu_join(cu_thread_t *thr, cu_reactor_t *reactor) {
    ++reactor->threads;
    g_assert(thr->coro == NULL);
    thr->coro = cu_reactor_get_current_coro(reactor);
    reactor->caller = thr->coro->id;
    getcontext(&thr->coro->context);
    if (reactor->caller == thr->coro->id) {
        setcontext(&reactor->context);
    }
}