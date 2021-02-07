#include "async_event.h"


static int32_t id = 0;

void 
async_event_init(
    async_event_t *event,
    async_event_poller_t poller,
    async_reactor_t *reactor
) {
    event->reactor = reactor;
    event->status = EVENT_NOT_STARTED;
    event->poller = poller;
    event->id = ++id;
    event->listeners = g_array_new(FALSE, FALSE, sizeof(coroutine_t *));
}

void 
async_event_add_listener(
    async_event_t *event,
    coroutine_t *coro
) {
    g_array_append_val(event->listeners, coro);
}


void
async_yield_until(
    async_event_t *event
) {
    coroutine_t *coro = async_reactor_get_current_coro(event->reactor);

    if (event->status == EVENT_FINISHED) {
        g_warning("Coro[id = %d] yielded finished event(id = %d)", coro->id, event->id);
    }

    if (event->status == EVENT_NOT_STARTED) {
        async_event_poll(event);
    }

    async_event_add_listener(event, coro);
    event->reactor->caller = coro->id;
    getcontext(&coro->context);
    if (event->reactor->caller == coro->id) {
        setcontext(&event->reactor->context);
    }
}

static void
poll_coro(void *_args) {
    async_event_t *event = _args;
    event->poller(event, event->poller_args);
    event->status = EVENT_FINISHED;
    async_reactor_coro_exit(event->reactor);
}

void 
async_event_poll(
    async_event_t *event
) {
    if (event->status != EVENT_NOT_STARTED) {
        g_warning("Finished event(id = %d) is polled again\n", event->id);
    }
    event->status = EVENT_POLL;
    async_reactor_make_coro(event->reactor, poll_coro, event);
}

void 
async_event_destroy(
    async_event_t *event
) {
    for (size_t i = 0; i < event->listeners->len; ++i) {
        coroutine_t *listener = g_array_index(event->listeners, coroutine_t *, i);
        if (listener->status != CORO_DONE) {
            g_warning("Event(id = %d) destroyed before listener [id = %d] notify", event->id, listener->id);
        }
    }
    g_array_free(event->listeners, TRUE);
}