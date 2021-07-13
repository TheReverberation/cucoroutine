#pragma once


#include "channel.h"
#include "thread.h"
#include "coroutine.h"
#include "reactor.h"

extern cu_reactor_t default_reactor;

#define cu_init() { \
    coro_status_name_init__(); \
    cu_reactor_init(&default_reactor); \
}

#define cu_destroy() { \
    cu_threads_destroy__(); \
}
