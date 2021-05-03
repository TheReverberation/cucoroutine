#pragma once


#include "async_channel.h"
#include "async_event.h"
#include "async_reactor.h"
#include "coroutine.h"
#include "async_threads.h"

extern async_reactor_t default_reactor;

inline void 
aio_init() {
    coro_status_name_init();
    async_reactor_init(&default_reactor);
    async_threads_init();
}

#define aio_make_coro(f, a) async_reactor_make_coro(&default_reactor, f, a)
#define aio_run() async_reactor_run(&default_reactor)
#define aio_coro_exit() async_reactor_coro_exit(&default_reactor)
#define aio_yield_at_time(t) async_reactor_yield_at_time(&default_reactor, t)

#define aio_chan_open(cap) async_chan_open(&default_reactor, cap)

#define aio_event_init(event, poller) async_event_init(event, poller, &default_reactor)

#define aio_begin_compute() begin_compute(&default_reactor)
#define aio_end_compute() end_compute(&default_reactor)