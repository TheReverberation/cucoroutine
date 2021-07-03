#pragma once

#include <glib.h>

#include "async_event.h"
#include "reactor.h"
#include "cyclic_buffer.h"
#include "errors.h"


#define CU_ASYNC_CHAN_MAX_WRITERS 1024

typedef struct cu_async_chan {
    cu_cyclic_buffer_t buffer;
    cu_cyclic_buffer_t writers;
    bool notify_writers_ran;
    cu_coroutine_t *notify_writers;
    cu_coroutine_t *reader;
    cu_reactor_t *reactor;
    int32_t id;
} cu_async_chan_t;

cu_async_chan_t *
cu_async_chan_open(cu_reactor_t *reactor, uint32_t cap);

cu_err_t
cu_async_chan_send(cu_async_chan_t *chan, void *data);

void *
cu_async_chan_read(cu_async_chan_t *chan);

void 
cu_async_chan_close(cu_async_chan_t *chan);
