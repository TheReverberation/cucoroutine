#pragma once

#include <pthread.h>

#include "reactor.h"


void cu_begin_compute(cu_reactor_t *reactor);
void cu_end_compute(cu_reactor_t *reactor);
void cu_threads_init__();
void cu_threads_destroy__();
