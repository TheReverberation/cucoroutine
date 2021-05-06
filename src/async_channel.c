#include <glib.h>

#include "async_channel.h"


static void
notify_writers(
    void *_chan
) {
    cu_async_chan_t *chan = _chan;
    if (!cu_cyclic_buffer_is_empty(&chan->writers) && !cu_cyclic_buffer_is_full(&chan->writers)) {
        cu_reactor_add_coro(chan->reactor, (cu_coroutine_t *) cu_cyclic_buffer_pop(&chan->writers));
    }
    chan->notify_writers_ran = false;
}

static int32_t id = 0;

cu_async_chan_t *
cu_async_chan_open(
    cu_reactor_t *reactor, 
    uint32_t cap
) {
    cu_async_chan_t *chan = malloc(sizeof(cu_async_chan_t));
    if (!chan) {
        return NULL;
    }
    if (!cu_cyclic_buffer_init(&chan->buffer, cap)) {
        free(chan);
        return NULL;
    }
    if (!cu_cyclic_buffer_init(&chan->writers, CU_ASYNC_CHAN_MAX_WRITERS)) {
        cu_cyclic_buffer_destroy(&chan->buffer);
        free(chan);
        return NULL;
    }
    
    chan->notify_writers = cu_make(notify_writers, chan, reactor);
    chan->reader = NULL;
    chan->id = ++id;
    
    chan->reactor = reactor;
    chan->notify_writers_ran = false;
    return chan;
}

cu_err_t
cu_async_chan_send(
    cu_async_chan_t *chan, 
    void *data
) {
    cu_coroutine_t *coro = cu_reactor_get_current_coro(chan->reactor);
    if (cu_cyclic_buffer_is_full(&chan->buffer)) {
        cu_cyclic_buffer_push(&chan->writers, coro);
        chan->reactor->caller = coro->id;
        getcontext(&coro->context);
        if (chan->reactor->caller == coro->id) {
            setcontext(&chan->reactor->context);
        }
    }
    g_assert(!cu_cyclic_buffer_is_full(&chan->buffer));
    cu_cyclic_buffer_push(&chan->buffer, data);
    // nofity reader
    if (chan->reader) {
        cu_reactor_add_coro(chan->reactor, chan->reader);
        chan->reader = NULL;
    }
}



void *
cu_async_chan_read(
    cu_async_chan_t *chan
) {
    if (chan->buffer.size == 0) {
        cu_coroutine_t *coro = cu_reactor_get_current_coro(chan->reactor);
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

    if (!chan->notify_writers_ran) {
        chan->notify_writers_ran = true;
        _coro_goto_begin(chan->notify_writers, chan->reactor);
        cu_reactor_add_coro(chan->reactor, chan->notify_writers);
    }
    return data;
}

void 
cu_async_chan_close(
    cu_async_chan_t *chan
) {
    if (chan) {
        if (chan->writers.size > 0) {
            g_error("Chan[id = %d] is closed when some writers are waiting", chan->id);
        }
        cu_cyclic_buffer_destroy(&chan->writers);
        if (chan->buffer.size > 0) {
            g_warning("Chan[id = %d] is closed when data is not read", chan->id);
        }
        cu_cyclic_buffer_destroy(&chan->buffer);
        if (chan->reader) {
            g_error("Chan[id = %d] is closed when the reader is waiting", chan->id);
        }
        cu_coro_destroy(chan->notify_writers);
        free(chan->notify_writers);
    }
    free(chan);
}
