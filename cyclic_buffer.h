#pragma once

#include <stdbool.h>
#include <stdint.h>


typedef struct {
    void **data;
    uint32_t cap;
    uint32_t end;
    uint32_t begin;
    int32_t size;
} cyclic_buffer_t;

bool 
cyclic_buffer_init(
    cyclic_buffer_t *buffer,
    uint32_t cap
);


void 
cyclic_buffer_deinit(
    cyclic_buffer_t *buffer
);

void 
cyclic_buffer_push(
    cyclic_buffer_t *buffer,
    void *value
);

void *
cyclic_buffer_pop(
    cyclic_buffer_t *buffer
);

bool 
cyclic_buffer_is_empty(
    cyclic_buffer_t *buffer
);

bool
cyclic_buffer_is_full(
    cyclic_buffer_t *buffer
);


