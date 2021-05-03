#pragma once


typedef enum coro_status {
    CORO_NOT_EXEC,
    CORO_RUNNING,
    CORO_RUNNUNG_IN_THREAD,
    CORO_DONE,
    CORO_STATUS_END
} coro_status_t;

void 
coro_status_name_init();

char const *
coro_status_name(coro_status_t status);
