#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <ucontext.h>
#include <pthread.h>

#include <glib.h>

#include "coroutine.h"
#include "errors.h"

/// Cast milliseconds to nanoseconds
#define MS * 1000


/*!
 * Implements switching between coroutines according to time.
 * Automatically frees memory of coroutines maked with async_reactor_make_coro().
 */
typedef struct async_reactor {  
    GArray *maked_coros; // coroutine_t * array
    coroutine_t *current_coro;
    int caller;
    ucontext_t context;
    GTree *schedule; // tree of pair<guint64[2] = {runtime, coro.id}, coroutine_t>
    pthread_mutex_t mutex;
} async_reactor_t;

/*!
 * Initialize reactor.
 * \param[out] reactor - not null pointer to reactor
 * \return error code
 */
aio_err_t
async_reactor_init(
    async_reactor_t *reactor
);

/*!
 * Create new coroutine and add one to array for the next freeing. 
 * See #coro_make(), #async_reactor_run().
 */
void
async_reactor_make_coro(
    async_reactor_t *reactor,
    coro_func_t func,
    void *args
);

/*!
 * Add coro to schedule.
 * \param[out] reactor - pointer to reactor
 * \param[in] coro - coroutine pointer
 */
void
async_reactor_add_coro(
    async_reactor_t *reactor,
    coroutine_t *coro
);

/*!
 * Stop current coroutine, add one to schedule, switch to async_reactor_run().
 */
void
async_reactor_yield_at_time(
    async_reactor_t *reactor,
    guint64 run_after_u
);


void 
async_reactor_resume_coro(
    async_reactor_t *reactor
);

/*!
 * It's main function in library. Runs coroutines alternatively by schedule,
 * resume the nearest or sleep if it can.
 * Sometimes it unlocks its mutex so that threads can use reactor.
 * \return error code.
 */
aio_err_t 
async_reactor_run(
    async_reactor_t *reactor
);

coroutine_t *
async_reactor_get_current_coro(
    async_reactor_t *reactor
);

/*!
 * Exit from current coroutine.
 */
void
async_reactor_coro_exit(
    async_reactor_t *reactor
);


