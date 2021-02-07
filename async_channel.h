#pragma once

#include <glib.h>

#include "async_event.h"
#include "async_reactor.h"
#include "cyclic_buffer.h"

#define ASYNC_CHAN_MAX_WRITERS 1024

typedef struct async_chan {
    cyclic_buffer_t buffer;
    cyclic_buffer_t writers;
    bool notify_writers_ran;
    coroutine_t *notify_writers;
    coroutine_t *reader;   
    async_reactor_t *reactor;
    int32_t id;
} async_chan_t;

async_chan_t *
async_chan_open(async_reactor_t *reactor, uint32_t cap);

void
async_chan_send(async_chan_t *chan, void *data);

void *
async_chan_read(async_chan_t *chan);

void async_chan_close(async_chan_t *chan);