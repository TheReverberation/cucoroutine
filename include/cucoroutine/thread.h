/*!
 * \file
 * threading
 */
#pragma once

#include "reactor.h"

typedef struct cu_thread *cu_thread_t;

/*!
 * The function starts a new thread in calling process, executing by calling *func*.
 *
 * A thread must refer to reactor only by locking it's mutex. According to coroutine realisation of
 * concurrency, a thread must not interrupt reactor execution for many time.
 * So, a thread must not use variables concurrently with a some coroutine. It leads to __UB__.
 *
 * A thread can be joined, see #cu_join.
 * \param thr - pointer to thread
 * \param func - calling function
 * \param arg - argument passed to function
 * \return on success, return 0; on error, return error code and content of *thr* is undefined.
 */
cu_err_t cu_thread_create(cu_thread_t *thr, void (*func)(void *), void *arg);

/*!
 * Wait until a thread exit.
 * \param thr
 * \param reactor
 */
void cu_join(cu_thread_t thr, cu_reactor_t reactor);