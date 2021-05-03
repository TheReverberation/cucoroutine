#include <assert.h>
#include <stdlib.h>

#include <glib.h>

#include "cyclic_buffer.h"


bool 
cyclic_buffer_init(
    cyclic_buffer_t *buffer,
    uint32_t cap
) {
    buffer->begin = buffer->end = 0;
    buffer->size = 0;
    if ((buffer->data = malloc(cap * sizeof(void *)))) {
        buffer->cap = cap;
        return true;
    } else {
        return false;
    }
}


void 
cyclic_buffer_deinit(
    cyclic_buffer_t *buffer
) {
    free(buffer->data);
    buffer->data = NULL;
}

bool 
cyclic_buffer_is_empty(
    cyclic_buffer_t *buffer
) {
    return buffer->size == 0;
}

bool
cyclic_buffer_is_full(
    cyclic_buffer_t *buffer
) {
    return buffer->size == buffer->cap;
}

void 
cyclic_buffer_push(
    cyclic_buffer_t *buffer,
    void *value
) {
    g_assert(!cyclic_buffer_is_full(buffer));
    ++(buffer->size);
    buffer->data[buffer->end] = value;
    buffer->end = (buffer->end + 1) % buffer->cap;
}

void *
cyclic_buffer_pop(
    cyclic_buffer_t *buffer
) {
    g_assert(buffer->size > 0);
    void *ret = buffer->data[buffer->begin];
    buffer->begin = (buffer->begin + 1) % buffer->cap;
    --(buffer->size);
    return ret;
}

