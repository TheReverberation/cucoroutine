#include <assert.h>
#include <stdlib.h>

#include <glib.h>

#include "cyclic_buffer.h"


cu_err_t
cu_cyclic_buffer_init(
        cu_cyclic_buffer_t *buffer,
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
cu_cyclic_buffer_destroy(
        cu_cyclic_buffer_t *buffer
) {
    free(buffer->data);
    buffer->data = NULL;
}

bool 
cu_cyclic_buffer_is_empty(
        cu_cyclic_buffer_t *buffer
) {
    return buffer->size == 0;
}

bool
cu_cyclic_buffer_is_full(
        cu_cyclic_buffer_t *buffer
) {
    return buffer->size == buffer->cap;
}

void 
cu_cyclic_buffer_push(
        cu_cyclic_buffer_t *buffer,
        void *value
) {
    g_assert(!cu_cyclic_buffer_is_full(buffer));
    ++(buffer->size);
    buffer->data[buffer->end] = value;
    buffer->end = (buffer->end + 1) % buffer->cap;
}

void *
cu_cyclic_buffer_pop(
        cu_cyclic_buffer_t *buffer
) {
    g_assert(buffer->size > 0);
    void *ret = buffer->data[buffer->begin];
    buffer->begin = (buffer->begin + 1) % buffer->cap;
    --(buffer->size);
    return ret;
}

