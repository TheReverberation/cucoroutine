#include <glib.h>

#include "async_channel.h"


static void
notify_writers(
    void *_chan
) {
    async_chan_t *chan = _chan;
    if (!cyclic_buffer_is_empty(&chan->writers) && !cyclic_buffer_is_full(&chan->writers)) {
        async_reactor_add_coro(chan->reactor, (coroutine_t *)cyclic_buffer_pop(&chan->writers));
    }
    chan->notify_writers_ran = false;
}

static int32_t id = 0;

async_chan_t *
async_chan_open(
    async_reactor_t *reactor, 
    uint32_t cap
) {
    async_chan_t *chan = malloc(sizeof(async_chan_t));
    if (!chan) {
        return NULL;
    }
    if (!cyclic_buffer_init(&chan->buffer, cap)) {
        free(chan);
        return NULL;
    }
    if (!cyclic_buffer_init(&chan->writers, ASYNC_CHAN_MAX_WRITERS)) {
        cyclic_buffer_deinit(&chan->buffer);
        free(chan);
        return NULL;
    }
    
    chan->notify_writers = coro_make(notify_writers, chan, reactor);
    chan->reader = NULL;
    chan->id = ++id;
    
    chan->reactor = reactor;
    chan->notify_writers_ran = false;
    return chan;
}

aio_err_t
async_chan_send(
    async_chan_t *chan, 
    void *data
) {
    coroutine_t *coro = async_reactor_get_current_coro(chan->reactor);
    if (cyclic_buffer_is_full(&chan->buffer)) {
        cyclic_buffer_push(&chan->writers, coro);
        chan->reactor->caller = coro->id;
        getcontext(&coro->context);
        if (chan->reactor->caller == coro->id) {
            setcontext(&chan->reactor->context);
        }
    }
    g_assert(!cyclic_buffer_is_full(&chan->buffer));
    cyclic_buffer_push(&chan->buffer, data);
    // nofity reader
    if (chan->reader) {
        async_reactor_add_coro(chan->reactor, chan->reader);
        chan->reader = NULL;
    }
}



void *
async_chan_read(
    async_chan_t *chan
) {
    if (chan->buffer.size == 0) {
        coroutine_t *coro = async_reactor_get_current_coro(chan->reactor);
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
    void *data = cyclic_buffer_pop(&chan->buffer);

    if (!chan->notify_writers_ran) {
        chan->notify_writers_ran = true;
        _coro_goto_begin(chan->notify_writers, chan->reactor);
        async_reactor_add_coro(chan->reactor, chan->notify_writers);
    }
    return data;
}

void 
async_chan_close(
    async_chan_t *chan
) {
    if (chan) {
        if (chan->writers.size > 0) {
            g_error("Chan[id = %d] is closed when some writers are waiting", chan->id);
        }
        cyclic_buffer_deinit(&chan->writers);
        if (chan->buffer.size > 0) {
            g_warning("Chan[id = %d] is closed when data is not read", chan->id);
        }
        cyclic_buffer_deinit(&chan->buffer);
        if (chan->reader) {
            g_error("Chan[id = %d] is closed when the reader is waiting", chan->id);
        }
        coro_destroy(chan->notify_writers);
        free(chan->notify_writers);
    }
    free(chan);
}
