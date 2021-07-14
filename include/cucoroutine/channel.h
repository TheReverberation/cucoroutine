#pragma once

#include "reactor.h"

typedef struct cu_chan *cu_chan_t;

cu_chan_t
cu_async_chan_open(uint32_t cap, cu_reactor_t reactor);

cu_err_t
cu_async_chan_send(cu_chan_t chan, void *data);

void *
cu_async_chan_read(cu_chan_t chan);

void 
cu_async_chan_close(cu_chan_t chan);
