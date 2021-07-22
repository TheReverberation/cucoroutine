/*!
 * \file
 * \brief reactor operations
 */

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
 * Automatically frees memory of coroutines made with #cu_reactor_create_coro().
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
 * Create a new coroutine and add one to array for automatically freeing.
 * Composition of #cu_reactor_add_coro and #cu_create.
 * See #cu_create, #cu_reactor_run().
 * \param reactor
 * \param func - start function, see #cu_func_t
 * \param args - is passed to start function as the sole argument
 */
void
cu_reactor_create_coro(
    cu_reactor_t reactor,
    cu_func_t func,
    void *args
);

/*!
 * Add a coroutine to reactor.
 * Reactor runs coroutine from stopped place or begin it if it have not started yet.
 */
void
cu_reactor_add(
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
 * Destroys itself after.
 * \return The function always succeeds, returning 0.
 * \todo other errors
 */
cu_err_t 
cu_reactor_run(
    cu_reactor_t reactor
);

/*!
 * Returns pointer of the calling coroutine.
 * \param reactor
 * \return The function always succeeds, returning current running coroutine.
 */
cu_coroutine_t
cu_self(
    cu_reactor_t reactor
);

/*!
 * Exit from current coroutine.
 * The coroutine resources are not released.
 */
void
cu_exit(
    cu_reactor_t reactor
);


