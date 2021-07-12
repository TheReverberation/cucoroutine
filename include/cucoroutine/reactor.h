#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <ucontext.h>
#include <pthread.h>

#include <sys/epoll.h>

#include <glib.h>

#include "coroutine.h"
#include "errors.h"

/// Cast milliseconds to nanoseconds
#define MS * 1000

/*!
 * Implements switching between coroutines according to time.
 * Automatically frees memory of coroutines made with cu_reactor_make_coro().
 */
typedef struct cu_reactor {  
    GArray *coroutines; // cu_coroutine_t * array
    cu_coroutine_t *current_coro;
    int caller;
    ucontext_t context;
    GTree *schedule; // tree of pair<guint64[2] = {runtime, coro.id}, cu_coroutine_t>
    pthread_mutex_t mutex;
    pthread_cond_t thread_exit;
    int16_t threads;
    GTree *fd_dict; // tree of pair<int fd, GArray<cu_coroutine_t>>
    GArray *files;
    int epollfd;
    size_t files_cnt;
} cu_reactor_t;

/*!
 * Initialize reactor.
 * \param[out] reactor - not null pointer to reactor
 * \return error code
 */
cu_err_t
cu_reactor_init(
    cu_reactor_t *reactor
);

/*!
 * Create new coroutine and add one to array for the next freeing. 
 * See #coro_make(), #cu_reactor_run().
 */
void
cu_reactor_make_coro(
    cu_reactor_t *reactor,
    cu_func_t func,
    void *args
);

/*!
 * Add coro to schedule.
 * \param[out] reactor - pointer to reactor
 * \param[in] coro - coroutine pointer
 */
void
cu_reactor_add_coro(
    cu_reactor_t *reactor,
    cu_coroutine_t *coro
);

/*!
 * Stop current coroutine, add one to schedule, switch to cu_reactor_run().
 */
void
cu_reactor_yield_at_time(
    cu_reactor_t *reactor,
    int64_t run_after_u
);


void 
cu_reactor_resume_coro(
    cu_reactor_t *reactor
);

/*!
 * It's main function in library. Runs coroutines alternatively by schedule,
 * resume the nearest or sleep if it can.
 * Sometimes it unlocks its mutex so that threads can use reactor.
 * \return error code.
 */
cu_err_t 
cu_reactor_run(
    cu_reactor_t *reactor
);

cu_coroutine_t *
cu_reactor_get_current_coro(
    cu_reactor_t *reactor
);

/*!
 * Exit from current coroutine.
 */
void
cu_coro_exit(
    cu_reactor_t *reactor
);


