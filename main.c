#include <stdio.h>


#include <pthread.h>

#include "aio.h"
#include <unistd.h>
#include "errors.h"


void coro1(void *args) {
    for (int i = 0; i < 10000; ++i) {
        printf("coro1 running\n");
        aio_yield_at_time(500 MS);
    }
    
}

void coro_with_compute(void *arg) {
    printf("coro comp start tid = %d\n", gettid());
    aio_begin_compute();
    for (int i = 0; i < 10; ++i) {
        usleep(100000);
        printf("compute %d%% in tid = %d\n", i * 10, gettid());
    }
    aio_end_compute();
    printf("computed tid = %d\n", gettid());
}

#include <errno.h>

int main() {
    printf("debug ? = %d\n", AIO_DEBUG);
    g_tree_insert(NULL, NULL, NULL);
    printf("main tid = %d\n", gettid());
    aio_init();
    aio_make_coro(coro1, NULL);
    aio_make_coro(coro_with_compute, NULL);
    return 0;
}

