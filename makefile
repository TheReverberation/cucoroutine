all: main

CC = clang
CFLAGS = -Wall -std=c11 -lm  -g3

CFLAGS += `pkg-config --cflags glib-2.0`
CFLAGS += `pkg-config --libs glib-2.0`

main: main.c aio.o async_reactor.o coroutine.o coro_status.o
	$(CC) -o main main.c aio.o async_reactor.o coroutine.o coro_status.o -lpthread $(CFLAGS)

aio.o: aio.h aio.c 
	$(CC) -o aio.o -c aio.c $(CFLAGS)

async_reactor.o: async_reactor.h async_reactor.c 
	$(CC) -o async_reactor.o -c async_reactor.c  $(CFLAGS)

coroutine.o: coroutine.h coroutine.c 
	$(CC) -o coroutine.o -c coroutine.c  $(CFLAGS)

coro_status.o: coro_status.h coro_status.c 
	$(CC) -o coro_status.o -c coro_status.c $(CFLAGS)




	
