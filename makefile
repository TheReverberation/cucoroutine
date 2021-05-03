all: main




CC = clang
BUILD_TYPE = debug
CFLAGS = -Wall -std=c11 -O3

SOURCE_DIR = src
INCLUDE_DIR = include
BUILD_DIR = bin

CFLAGS += -I$(INCLUDE_DIR)

ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -g3 -pg -DDEBUG -DAIO_DEBUG
endif


# glib
CFLAGS += `pkg-config --cflags glib-2.0` 
GLIB = `pkg-config --libs glib-2.0`

SOURCES := $(wildcard $(SOURCE_DIR)/*.c) # src/foo.c
OBJECTS := $(SOURCES:$(SOURCE_DIR)/%=$(BUILD_DIR)/%.o) # src/foo.c -> build/foo.c.o

main: main.c libcucoroutine.a
	$(CC) -o main main.c -lcucoroutine -L. -lpthread $(GLIB) $(CFLAGS)

libcucoroutine.a: $(OBJECTS)
	echo $^
	ar rcs libcucoroutine.a $^
	ranlib libcucoroutine.a

# 
$(OBJECTS):$(BUILD_DIR)/%.o:$(SOURCE_DIR)/%
	echo $@:$?
	$(CC) -o $@ -c $? $(CFLAGS)

clean:
	rm *.o

rebuild: clean main
