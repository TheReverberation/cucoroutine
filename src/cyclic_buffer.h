#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "errors.h"

typedef struct {
    void **data;
    uint32_t cap;
    uint32_t end;
    uint32_t begin;
    int32_t size;
} cu_cyclic_buffer_t;

cu_err_t
cu_cyclic_buffer_init(
    cu_cyclic_buffer_t *buffer,
    int32_t cap
);


void 
cu_cyclic_buffer_destroy(
    cu_cyclic_buffer_t *buffer
);


void 
cu_cyclic_buffer_push(
    cu_cyclic_buffer_t *buffer,
    void *value
);

void *
cu_cyclic_buffer_pop(
    cu_cyclic_buffer_t *buffer
);

bool 
cu_cyclic_buffer_empty(
    cu_cyclic_buffer_t *buffer
);


bool
cu_cyclic_buffer_full(
    cu_cyclic_buffer_t *buffer
);


