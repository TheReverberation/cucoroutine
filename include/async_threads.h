#pragma once

#include <pthread.h>

#include "async_reactor.h"


void begin_compute(async_reactor_t *reactor);
void end_compute(async_reactor_t *reactor);
void async_threads_init();
