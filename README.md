# CUcoroutine
__CUcoroutine__ is a simply to use *ucontext.h* realisation of coroutine concurrency.

#### Hello, world!
```c

#include <stdio.h>

#include <cucoroutine/all.h>

void hello_world(void *arg) {
    printf("Hello, ");
    cu_yield_at_time(1000 MS, default_reactor);
    printf("world!\n");
}

int main() {
    cu_reactor_init(&default_reactor);
    cu_reactor_create_coro(default_reactor, hello_world, NULL);
    cu_reactor_run(default_reactor);
    return 0;
}
```

CUCoroutine provides APIs to:
- run coroutines concurrently
- perform I/O operations
- internal coroutine communication
- perform socket operations
- threading execution

## Basics
**Coroutine** is a function can save its condition(called __context__) and return to it after some time. A coroutine must have its own stack, save registers and signals, "*ucontext.h*" library uses for it.

Coroutine mustn't interrupt execution of reactor for many time. It should execute lightweight operations and yield execution by library's function.
Coroutine can calls a function that yield to another. 
All coroutines must be finished.

**Reactor** is a coroutine launcher, scheduler, event poller and I/O handler. 

## Requirements
- glib-2.0
- linux supported ucontext.h

## Examples
See examples directory.
## Docs
See cucoroutine/docs directory.
