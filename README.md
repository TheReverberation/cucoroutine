# CUcoroutine
CUcoroutine(C + ucontext + coroutine) is a **ucontext.h** realisation of coroutine concurrency.

## Sample

```c 
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
        cu_yield_at_time(25);
    }
}

void writer2(void *data) {
    for (size_t i = 0; i < 5000; ++i) {
        int *n = malloc(sizeof(int));
        *n = 2 * i + 1;
        async_chan_send(chan, n);
        cu_yield_at_time(50);
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
    cu_make_coro(writer1, NULL);
    cu_make_coro(reader, NULL);
    cu_make_coro(writer2, NULL);
}

int main() {
    cu_init();
    cu_make_coro(coro_main, NULL);
    cu_run();
    return 0;
}
```

## How it works
### Coroutine type
**Coroutine** is a function can save its condition(called __context__) and return to it after some time. A coroutine must have its own stack, save registers and signals, "*ucontext.h*" library uses for it. 

Coroutine doesn't interrupt execution of reactor for many time. It should execute lightweight operations or yield to other coroutines or yield until event happen.
Coroutine can calls a function that yield to another.
All coroutines must be finished.
```c
typedef void (*coro_func_t)(void *);

typedef struct coroutine {
    coro_func_t func;
    void *args;
    enum coro_status status;
    ucontext_t context;
    void *stack;
    int32_t id;
} coroutine_t;
```
### Async reactor

**Async reactor** is a struct manage execution of coroutines. It uses schedule tree contains pair of coroutine runtime and coroutine id as a key and coroutine pointer as a value.

Reactor select the nearest time to run a coroutine from the schedule and waits until the time comes, then reactor removes the coroutine from the schedule and returns to stopped coroutine.

One coroutine execute on the time only. You don't have to worry about the syncing.

```c
typedef struct async_reactor {  
    GArray *made_coros; // coroutine_t * array
    coroutine_t *current_coro;
    int caller;
    ucontext_t context;
    GTree *schedule; // tree of pair<guint64[2] = {runtime, coro.id}, coroutine_t>
} async_reactor_t;
```

### Async event system
**Async event** poll for occurence something event and notify its listeners.
```c
typedef void (*async_event_poller_t)(struct async_event *event, void *args);

typedef struct async_event {
    async_event_poller_t poller;
    void *poller_args;
    GArray *listeners; // coroutine
    void *event_data;
    enum event_status status;
    async_reactor_t *reactor;
    int32_t id;
} async_event_t;
```
It uses for more usability and faster schedule management.
The poll coroutine check the event occurrence instead direct event polling by coroutines.

Every time when a coroutine yields until an event, the event adds the coroutine to the array of listeners and continues it when the event occurs.


### Async channels
**Async channels** are data channels used to automatic non-blocking I/O.
```c
typedef struct async_chan {
    cu_cyclic_buffer_t buffer;
    cu_cyclic_buffer_t writers;
    bool notify_writers_ran;
    coroutine_t *notify_writers;
    coroutine_t *reader;   
    async_reactor_t *reactor;
    int32_t id;
} async_chan_t;
```
Async channel has one reader and one or more writers.

When the reader try to read from empty channel it yield until some writer send data to the channel.

Also when the writer try to send a data to full channel it yield until the reader get data from the channel.
