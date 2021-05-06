cmake_minimum_required(VERSION 3.19)
project(cucoroutine C)

set(CMAKE_C_STANDARD 11)
FIND_PACKAGE(PkgConfig REQUIRED)

PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)

FILE(GLOB SOURCES src/*.c)
FILE(GLOB HEADERS include/cucoroutine/*.h)

add_library(cucoroutine STATIC ${SOURCES} ${HEADERS})

target_include_directories(cucoroutine PUBLIC include/cucoroutine/ PUBLIC ${GLIB_INCLUDE_DIRS})
target_link_directories(cucoroutine PUBLIC ${GLIB_LIBRARY_DIRS})
target_link_libraries(cucoroutine ${GLIB_LIBRARIES} pthread)

set(CMAKE_BINARY_DIR bin)