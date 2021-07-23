#include <glib.h>
#include <stdio.h>
#include "coroutine_private.h"
#include "channel.h"
#include "util/cyclic_buffer.h"
#include "errors.h"
#include "reactor_private.h"

#define CU_ASYNC_CHAN_MAX_WRITERS 1024

struct cu_chan {
    cu_cyclic_buffer_t buffer;
    cu_cyclic_buffer_t writers;
    cu_coroutine_t notify_writers;
    cu_coroutine_t reader;
    cu_reactor_t reactor;
    int32_t id;
};

static int32_t id = 0;

struct cu_chan *
cu_async_chan_open(
    uint32_t cap,
    struct cu_reactor *reactor
) {
    struct cu_chan *chan = malloc(sizeof(struct cu_chan));
    if (!chan) {
        goto CHAN_CLEANUP;
    }
    if (cu_cyclic_buffer_init(&chan->buffer, cap) != 0) {
        goto BUFFER_CLEANUP;
    }
    if (cu_cyclic_buffer_init(&chan->writers, CU_ASYNC_CHAN_MAX_WRITERS) != 0) {
        goto WRITERS_CLEANUP;
    }
    
    chan->reader = NULL;
    chan->id = ++id;
    chan->reactor = reactor;
    return chan;

WRITERS_CLEANUP:
    cu_cyclic_buffer_destroy(&chan->buffer);
BUFFER_CLEANUP:
    free(chan);
CHAN_CLEANUP:
    cu_seterr(ENOMEM);
    return NULL;
}

void
cu_async_chan_write(
    struct cu_chan *chan,
    void *data
) {
    cu_coroutine_t coro = cu_self(chan->reactor);
    if (cu_cyclic_buffer_full(&chan->buffer)) {
        cu_cyclic_buffer_push(&chan->writers, coro);
        chan->reactor->caller = coro->id;
        getcontext(&coro->context);
        if (chan->reactor->caller == coro->id) {
            setcontext(&chan->reactor->context);
        }
    }
    g_assert(!cu_cyclic_buffer_full(&chan->buffer));
    cu_cyclic_buffer_push(&chan->buffer, data);
    // nofity reader
    if (chan->reader) {
        cu_reactor_add(chan->reactor, chan->reader);
        chan->reader = NULL;
    }
}



void *
cu_async_chan_read(
    struct cu_chan *chan
) {
    if (chan->buffer.size == 0) {
        cu_coroutine_t coro = cu_self(chan->reactor);
        if (chan->reader) {
            g_error("Chan[id = %d] has more that one readers\n", chan->id);
        }
        chan->reader = coro;
        chan->reactor->caller = coro->id;
        getcontext(&coro->context);
        if (chan->reactor->caller == coro->id) {
            setcontext(&chan->reactor->context);
        }
    }
    void *data = cu_cyclic_buffer_pop(&chan->buffer);

    if (!cu_cyclic_buffer_empty(&chan->writers) && !cu_cyclic_buffer_full(&chan->buffer)) {
        cu_reactor_add(chan->reactor, (cu_coroutine_t) cu_cyclic_buffer_pop(&chan->writers));
    }
    return data;
}

void 
cu_async_chan_close(
    struct cu_chan *chan
) {
    if (chan) {
//        if (chan->writers.size > 0) {
//            g_error("Chan[id = %d] is closed when some writers are waiting", chan->id);
//        }
        cu_cyclic_buffer_destroy(&chan->writers);
//        if (chan->buffer.size > 0) {
//            g_warning("Chan[id = %d] is closed when data is not read", chan->id);
//        }
        cu_cyclic_buffer_destroy(&chan->buffer);
        if (chan->reader) {
            g_error("Chan[id = %d] is closed when the reader is waiting", chan->id);
        }
    }
    free(chan);
}
