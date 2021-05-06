#include "coro_status.h"

static char const *coro_status_names[CORO_STATUS_END];

void _coro_status_name_init() {
    coro_status_names[CORO_NOT_EXEC] = "not exec";
    coro_status_names[CORO_RUNNING] = "running";
    coro_status_names[CORO_DONE] = "done";
}


char const *
coro_status_name(
    cu_coro_status_t status
) {
    return coro_status_names[status];
}


