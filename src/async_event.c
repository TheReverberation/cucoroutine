#include "async_event.h"


static int32_t id = 0;

void 
cu_async_event_init(
    cu_async_event_t *event,
    cu_async_event_poller_t poller,
    cu_reactor_t *reactor
) {
    event->reactor = reactor;
    event->status = EVENT_NOT_STARTED;
    event->poller = poller;
    event->id = ++id;
    event->listeners = g_array_new(FALSE, FALSE, sizeof(cu_coroutine_t *));
}

void 
cu_async_event_add_listener(
    cu_async_event_t *event,
    cu_coroutine_t *coro
) {
    g_array_append_val(event->listeners, coro);
}


void
cu_yield_until(
    cu_async_event_t *event
) {
    cu_coroutine_t *coro = cu_reactor_get_current_coro(event->reactor);

    if (event->status == EVENT_FINISHED) {
        g_warning("Coro[id = %d] yielded finished event(id = %d)", coro->id, event->id);
    }

    if (event->status == EVENT_NOT_STARTED) {
        cu_async_event_poll(event);
    }

    cu_async_event_add_listener(event, coro);
    event->reactor->caller = coro->id;
    getcontext(&coro->context);
    if (event->reactor->caller == coro->id) {
        setcontext(&event->reactor->context);
    }
}

static void
poll_coro(void *_args) {
    cu_async_event_t *event = _args;
    event->poller(event, event->poller_args);
    event->status = EVENT_FINISHED;
    cu_reactor_coro_exit(event->reactor);
}

void 
cu_async_event_poll(
    cu_async_event_t *event
) {
    if (event->status != EVENT_NOT_STARTED) {
        g_warning("Finished event(id = %d) is polled again\n", event->id);
    }
    event->status = EVENT_POLL;
    cu_reactor_make_coro(event->reactor, poll_coro, event);
}

void 
cu_async_event_destroy(
    cu_async_event_t *event
) {
    for (size_t i = 0; i < event->listeners->len; ++i) {
        cu_coroutine_t *listener = g_array_index(event->listeners, cu_coroutine_t *, i);
        if (listener->status != CORO_DONE) {
            g_warning("Event(id = %d) destroyed before listener [id = %d] notify", event->id, listener->id);
        }
    }
    g_array_free(event->listeners, TRUE);
}