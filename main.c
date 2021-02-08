#include <stdio.h>

#include "aio.h"
#include "async_event.h"
#include "async_channel.h"


async_chan_t *chan;


void writer1(void *data) {
    printf("writer1 isd %d\n", async_reactor_get_current_coro(&default_reactor)->id);
    for (size_t i = 0; i < 5000; ++i) {
        int *n = malloc(sizeof(int));
        *n = 2 * i;
        async_chan_send(chan, n);
        aio_yield_at_time(25);
    }
}

void writer2(void *data) {
    for (size_t i = 0; i < 5000; ++i) {
        int *n = malloc(sizeof(int));
        *n = 2 * i + 1;
        async_chan_send(chan, n);
        aio_yield_at_time(50);
    }
}


void reader(void *data) {
    size_t i = 0;
    int *n = async_chan_read(chan);
    ++i;
    for (; i < 10000; ++i) {
        printf("%d^2 %d\n", *n, *n * *n);
        free(n);
        n = async_chan_read(chan);
    }
    free(n);
    async_chan_close(chan);
}


void coro_main(void *arg) {
    chan = async_chan_open(&default_reactor, 100);
    aio_make_coro(writer1, NULL);
    aio_make_coro(reader, NULL);
    aio_make_coro(writer2, NULL);
}

int main() {
    aio_init();
    aio_make_coro(coro_main, NULL);
    aio_run();
    return 0;
}

