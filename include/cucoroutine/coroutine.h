/*!
 * \file
 * \brief coroutine operations
 */
#pragma once

#include "errors.h"


typedef struct cu_reactor *cu_reactor_t;

typedef void (*cu_func_t)(void *);
typedef struct cu_coroutine *cu_coroutine_t;

/*!
 * Create a new coroutine on automatically allocated memory. Used to for manual store of coroutines.
 * After usage, #cu_coro_destroy must be called.
 * \param func - start function, see #cu_func_t
 * \param args - is passed to start function as the sole argument
 * \param reactor
 * \return on error, returns NULL, and error set appropriately, see errors.h.
 */
cu_coroutine_t
cu_create(
    cu_func_t func,
    void *args,
    cu_reactor_t reactor
);


/*!
 * Destroys initialized coroutine.
 * @param coro - coroutine.
 */
void
cu_coro_destroy(cu_coroutine_t coro);


void
coro_status_name_init__();
