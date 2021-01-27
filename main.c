#include <stdio.h>
#include <stdlib.h>

#include "aio.h"


int coro_calls = 0;

int coro2() {
    //exit(-1);
    int calls = coro_calls;
    printf("coro2: %d 1\n", calls);
    aio_coro_yield(1000);
    printf("coro2: %d 2\n", calls);
    return calls;
    //aio_coro_exit();
}


void coro1(void *_) {
    int calls = ++coro_calls;
    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 100 * i; j < 100 * (i + 1); ++j) {
            printf("coro1: %d %ld\n", calls, j);
        }
        printf("coro2 return %d\n", coro2());
        //aio_add_coro(coro_make(coro2));
        aio_coro_yield(1000);
    }
    aio_coro_exit();
}


int main() {
    aio_init();
    for (size_t i = 0; i < 1024; ++i) {
        aio_add_coro(coro1, NULL);
    }
    aio_run();
    return 0;
}

