#pragma once

#include <assert.h>
#include <stdbool.h>


#include <glib.h>

#include "coroutine.h"
#include "reactor.h"

struct async_event;
typedef void (*cu_async_event_poller_t)(struct async_event *event, void *args);

enum event_status {
    EVENT_NOT_STARTED,
    EVENT_POLL,
    EVENT_FINISHED
};

typedef struct async_event {
    cu_async_event_poller_t poller;
    void *poller_args;
    GArray *listeners; // coroutine
    void *event_data;
    enum event_status status;
    cu_reactor_t *reactor;
    int32_t id;
} cu_async_event_t;

void 
cu_async_event_init(
    cu_async_event_t *event,
    cu_async_event_poller_t poller,
    cu_reactor_t *reactor
);

void 
cu_async_event_add_listener(
    cu_async_event_t *event,
    cu_coroutine_t *coro
);

void 
cu_yield_until(
    cu_async_event_t *event
);


void 
cu_async_event_poll(
    cu_async_event_t *event
);

void 
cu_async_event_destroy(
    cu_async_event_t *event
);


