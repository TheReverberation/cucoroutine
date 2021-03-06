#pragma once

#include "reactor.h"

typedef struct cu_thread *cu_thread_t;

cu_err_t cu_thread_create(cu_thread_t *thr, void (*func)(void *), void *arg);
cu_err_t cu_join(cu_thread_t thr, cu_reactor_t reactor);