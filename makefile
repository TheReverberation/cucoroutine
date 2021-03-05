all: main

CC = clang
BUILD_TYPE = debug
CFLAGS = -Wall -std=c11 -O3

ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -g3 -pg
endif

CFLAGS += `pkg-config --cflags glib-2.0`
GLIB = `pkg-config --libs glib-2.0`

main: main.c libcucoroutine.a
	$(CC) -o main main.c -L. -lcucoroutine $(GLIB) $(CFLAGS)

libcucoroutine.a: aio.o async_reactor.o coroutine.o coro_status.o async_event.o cyclic_buffer.o async_channel.o
	ar ru libcucoroutine.a aio.o async_reactor.o coroutine.o coro_status.o async_event.o cyclic_buffer.o async_channel.o
	ranlib libcucoroutine.a



aio.o: aio.h aio.c 
	$(CC) -o aio.o -c aio.c $(CFLAGS)

async_reactor.o: async_reactor.h async_reactor.c   
	$(CC) -o async_reactor.o -c async_reactor.c $(CFLAGS)

coroutine.o: coroutine.h coroutine.c 
	$(CC) -o coroutine.o -c coroutine.c $(CFLAGS)

coro_status.o: coro_status.h coro_status.c 
	$(CC) -o coro_status.o -c coro_status.c $(CFLAGS)

async_event.o: async_event.h async_event.c 
	$(CC) -o async_event.o -c async_event.c $(CFLAGS)

cyclic_buffer.o: cyclic_buffer.c cyclic_buffer.h
	$(CC) -o cyclic_buffer.o -c cyclic_buffer.c $(CFLAGS)

async_channel.o: async_channel.h async_channel.c 
	$(CC) -o async_channel.o -c async_channel.c $(CFLAGS)

clean:
	rm *.o

rebuild: clean main
