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
typedef struct cu_reactor *cu_reactor_t;

/*!
 * Initialize reactor.
 * \param[out] reactor - not null pointer to reactor
 * \return error code. Zero is success code.
 * Error depends on reactor members init functions. This is forwarding from them.
 */
cu_err_t
cu_reactor_init(
    cu_reactor_t *reactor
);

/*!
 * Create a new coroutine and add one to array for the next freeing.
 * See #cu_coro_make(), #cu_reactor_run().
 */
void
cu_reactor_make_coro(
    cu_reactor_t reactor,
    cu_func_t func,
    void *args
);

/*!
 * Add a coroutine to schedule.
 */
void
cu_reactor_add_coro(
    cu_reactor_t reactor,
    cu_coroutine_t coro
);

/*!
 * Stops the current coroutine and continues after at least the specified time.
 * \param run_after_u - delay in nanoseconds
 * \param reactor - reactor
 */
void
cu_yield_at_time(
    int64_t run_after_u,
    cu_reactor_t reactor
);

/*!
 * Runs coroutines alternatively by schedule, io events, thread events.
 * \return 0.
 * \todo other errors
 */
cu_err_t 
cu_reactor_run(
    cu_reactor_t reactor
);

/*!
 *
 * \param reactor
 * \return current running coroutine
 */
cu_coroutine_t
cu_self(
    cu_reactor_t reactor
);

/*!
 * Exit from current coroutine.
 */
void
cu_exit(
    cu_reactor_t reactor
);


