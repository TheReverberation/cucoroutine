#pragma once

#include <assert.h>
#include <stdbool.h>


#include <glib.h>

#include "coroutine.h"
#include "async_reactor.h"

struct async_event;
typedef void (*async_event_poller_t)(struct async_event *event, void *args);

enum event_status {
    EVENT_NOT_STARTED,
    EVENT_POLL,
    EVENT_FINISHED
};

typedef struct async_event {
    async_event_poller_t poller;
    void *poller_args;
    GArray *listeners; // coroutine
    void *event_data;
    enum event_status status;
    async_reactor_t *reactor;
    int32_t id;
} async_event_t;

void 
async_event_init(
    async_event_t *event,
    async_event_poller_t poller,
    async_reactor_t *reactor
);

void 
async_event_add_listener(
    async_event_t *event,
    coroutine_t *coro
);

void 
async_yield_until(
    async_event_t *event
);


void 
async_event_poll(
    async_event_t *event
);

void 
async_event_destroy(
    async_event_t *event
);


