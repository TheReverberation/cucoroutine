#pragma once


#include "async_channel.h"
#include "async_event.h"
#include "reactor.h"
#include "coroutine.h"
#include "async_threads.h"

extern cu_reactor_t default_reactor;

inline void 
cu_init() {
    _coro_status_name_init();
    cu_reactor_init(&default_reactor);
    _cu_threads_init();
}
