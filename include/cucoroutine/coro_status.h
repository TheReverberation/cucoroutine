#pragma once


typedef enum coro_status {
    CORO_NOT_EXEC,
    CORO_RUNNING,
    CORO_RUNNUNG_IN_THREAD,
    CORO_DONE,
    CORO_STATUS_END
} cu_coro_status_t;

void 
_coro_status_name_init();

char const *
coro_status_name(cu_coro_status_t status);
