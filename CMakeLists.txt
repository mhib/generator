cmake_minimum_required(VERSION 3.3)
project(generator)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES main.c post.h sdsalloc.h sds.h sds.c)
add_executable(generator ${SOURCE_FILES})