#include <stdio.h>

#include <cucoroutine/all.h>


struct fibmode_args {
    uint32_t n, mod;
    uint32_t result;
};

void fibmode(void *arg_) {
    struct fibmode_args *args = arg_;
    uint64_t a = 0, b = 1;

    uint64_t mod = args->mod;
    if (args->n == 0) {
        args->result = 0;
        return;
    }
    for (int32_t i = 1; i < args->n; ++i) {
        uint64_t temp = a;
        a = b;
        b = (b + temp) % mod;
    }
    args->result = b;
}

void producer(void *chan_) {
    cu_chan_t chan  = chan_;
    for (size_t i = 0; i < (int)1e2; ++i) {
        int *n = malloc(sizeof(int));
        *n = rand() % (int)(1e7L) ;
        cu_async_chan_write(chan, n);
        cu_yield_at_time(100 MS, default_reactor);
    }
    int *n = malloc(sizeof(int));
    *n = -1;
    cu_async_chan_write(chan, n);
}

void consumer(void *chan_) {
    cu_chan_t chan = chan_;
    int *n = cu_async_chan_read(chan);
    while (*n != -1) {
        cu_thread_t thr;
        struct fibmode_args args = {.n = *n, .mod = rand()};
        cu_thread_create(&thr, fibmode, &args);
        cu_join(thr, default_reactor);
        printf("fib(%u) mod %u = %d\n", *n, args.mod, args.result);
        free(n);
        n = cu_async_chan_read(chan);
    }
    cu_async_chan_close(chan);
}

int main() {
    cu_reactor_init(&default_reactor);
    cu_chan_t chan = cu_async_chan_open(1, default_reactor);
    cu_reactor_create_coro(default_reactor, producer, chan);
    cu_reactor_create_coro(default_reactor, consumer, chan);
    cu_reactor_run(default_reactor);
    return 0;
}