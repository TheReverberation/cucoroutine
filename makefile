all: main


CC = clang
BUILD_TYPE = debug
CFLAGS = -Wall -std=c11 -O3

SOURCE_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

CFLAGS += -I$(INCLUDE_DIR)

ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -g3 -pg -DDEBUG -DAIO_DEBUG
endif


# glib
CFLAGS += `pkg-config --cflags glib-2.0` 
GLIB = `pkg-config --libs glib-2.0`

SOURCES := $(wildcard $(SOURCE_DIR)/*.c) # src/foo.c
OBJECTS := $(SOURCES:$(SOURCE_DIR)/%=$(BUILD_DIR)/%.o) # src/foo.c -> build/foo.c.o

main: main.c lib/libcucoroutine.a
	$(CC) -o main main.c -lcucoroutine -Llib/ -lpthread $(GLIB) $(CFLAGS)

lib/libcucoroutine.a: $(OBJECTS)
	ar rcs lib/libcucoroutine.a $^
	ranlib lib/libcucoroutine.a

# 
$(OBJECTS):$(BUILD_DIR)/%.o:$(SOURCE_DIR)/%
	$(CC) -o $@ -c $? $(CFLAGS)

clean:
	rm -r -f build/ lib/
	mkdir build
	mkdir lib
rebuild: clean main
