/*!
 * \file
 * \brief data channels
 *
 * Async data channels with automatic blocking of coroutine during read/write operations.
 */

#pragma once

#include "reactor.h"

typedef struct cu_chan *cu_chan_t;

/*!
 * Opens a new channel with buffer capacity of *cap* bytes.
 * @param cap - natural buffer capacity in bytes
 * @param reactor
 * @return Upon successful creating returns struct cu_chan pointer.
 * Otherwise, NULL is returned and error set appropriately.
 * ### Errors
 * - ENOMEM - no memory for specified buffer capacity, malloc() returned NULL.
 */
cu_chan_t
cu_async_chan_open(uint32_t cap, cu_reactor_t reactor);

/*!
 * Blocks until queue is full.
 * Pushes *data* pointer to channel queue.
 */
void
cu_async_chan_write(cu_chan_t chan, void *data);

/*!
 * Blocks until channel's queue is empty.
 * Pops data from channel queue and returns it.
 * @param chan
 * @return data
 */
void *
cu_async_chan_read(cu_chan_t chan);

/*!
 * Closes channel.
 * @param chan
 */
void 
cu_async_chan_close(cu_chan_t chan);
