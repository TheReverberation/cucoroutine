#include <stdio.h>
#include <stdlib.h>

#include "aio.h"
//#include <ucontext.h>

/*
ucontext_t foocontext, maincontext;
int curfunc = 0;
int arg;

void foo() {
    printf("foo begin\n");
    int i = 0;
    while (arg != -1) {
        printf("foo: %d %d\n", arg, i * i);
        ++i;
        curfunc = 1;
        getcontext(&foocontext);
        if (curfunc == 1) {
            setcontext(&maincontext);
        }
    }
    printf("foo end\n");
}

int main() {
    printf("main begin\n");
    curfunc = 0;
    getcontext(&maincontext);
    if (curfunc == 0) {
        foo();
    }
    for (size_t i = 0; i < 10; ++i) {
        curfunc = 0;
        getcontext(&maincontext);
        if (curfunc == 0) {
            arg = i;
            setcontext(&foocontext);
        }
    }
    curfunc = 0;
    getcontext(&maincontext);
    if (curfunc == 0) {
        arg = -1;
        setcontext(&foocontext);
    }
    printf("main end\n");
    return 0;
}*/

int coro_calls = 0;

void coro2() {
    //exit(-1);
    int calls = coro_calls;
    printf("coro2: %d 1\n", calls);
    aio_coro_yield();
    printf("coro2: %d 2\n", calls);
    aio_coro_exit();
}


void coro1() {
    int calls = ++coro_calls;
    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 100 * i; j < 100 * (i + 1); ++j) {
            printf("coro1: %d %ld\n", calls, j);
        }
        aio_add_coro(coro_make(coro2));
        aio_coro_yield();
    }
    aio_coro_exit();
}

int main() {
    aio_init();
    for (size_t i = 0; i < 1024; ++i) {
        aio_add_coro(coro_make(coro1));
    }
    aio_run();
    return 0;
}