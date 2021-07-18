#pragma once

typedef struct cu_reactor {
    GArray *coroutines; // cu_coroutine_t array
    cu_coroutine_t current_coro;
    int caller;
    ucontext_t context;
    GTree *schedule; // tree of pair<guint64[2] = {runtime, coro.id}, cu_coroutine_t>
    pthread_mutex_t mutex;
    pthread_cond_t thread_exit;
    int16_t threads;
    GTree *fd_dict; // tree of pair<int fd, GArray<cu_coroutine_t>>
    GArray *files;
    int epollfd;
    size_t files_cnt;
} *cu_reactor_t;

void
cu_reactor_resume_coro(
    cu_reactor_t reactor
);