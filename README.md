# CUcoroutine
CUcoroutine(C + ucontext + coroutine) is a simply to use **ucontext.h** realisation of coroutine concurrency.

```c 

```
### Coroutine 
**Coroutine** is a function can save its condition(called __context__) and return to it after some time. A coroutine must have its own stack, save registers and signals, "*ucontext.h*" library uses for it.

Coroutine doesn't interrupt execution of reactor for many time. It should execute lightweight operations and yield execution by library's function.
Coroutine can calls a function that yield to another.
All coroutines must be finished.
## Requirements
glib-2.0

## Examples
See examples directory.
## Docs
See cucoroutine/docs directory.
