all: main

CC = clang
CFLAGS = -Wall -std=c11 -lm -O3 -g3

main: main.c aio.o async_reactor.o coroutine.o coro_status.o run_on_stack.o
	$(CC) -o main main.c aio.o async_reactor.o coroutine.o coro_status.o run_on_stack.o $(CFLAGS)

aio.o: aio.h aio.c 
	$(CC) -o aio.o -c aio.c $(CFLAGS)

async_reactor.o: async_reactor.h async_reactor.c 
	$(CC) -o async_reactor.o -c async_reactor.c  $(CFLAGS)

coroutine.o: coroutine.h coroutine.c 
	$(CC) -o coroutine.o -c coroutine.c  $(CFLAGS)

coro_status.o: coro_status.h coro_status.c 
	$(CC) -o coro_status.o -c coro_status.c $(CFLAGS)

run_on_stack.o: run_on_stack.h run_on_stack.asm
	nasm -f elf64 -g -F dwarf -o run_on_stack.o run_on_stack.asm



	
