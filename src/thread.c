#include <stdio.h>
#include <pthread.h>

#include <glib.h>

#include "coroutine_private.h"
#include "thread.h"
#include "errors.h"
#include "reactor.h"
#include "reactor_private.h"

typedef struct cu_thread {
    void (*func)(void *);
    void *arg;
    cu_coroutine_t coro;
} *cu_thread_t;


static void *
run(void *thread_) {
    cu_thread_t thread = *(cu_thread_t *)thread_;
    thread->func(thread->arg);
    if (thread->coro != NULL) {
        pthread_mutex_lock(&thread->coro->reactor->mutex);
        cu_reactor_add(thread->coro->reactor, thread->coro);
        --thread->coro->reactor->threads;
        pthread_cond_signal(&thread->coro->reactor->thread_exit);
        pthread_mutex_unlock(&thread->coro->reactor->mutex);
    } else {
        g_assert(false);
    }
    return NULL;
}

cu_err_t cu_thread_create(cu_thread_t *thr, void (*func)(void *), void *arg) {
    *thr = malloc(sizeof(struct cu_thread));
    (*thr)->func = func;
    (*thr)->arg = arg;
    (*thr)->coro = NULL;
    pthread_t pthr;
    return pthread_create(&pthr, NULL, run, thr);
}

void cu_join(cu_thread_t thr, cu_reactor_t reactor) {
    ++reactor->threads;
    g_assert(thr->coro == NULL);
    thr->coro = cu_self(reactor);
    reactor->caller = thr->coro->id;
    getcontext(&thr->coro->context);
    if (reactor->caller == thr->coro->id) {
        setcontext(&reactor->context);
    }
}