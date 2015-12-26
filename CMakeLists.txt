cmake_minimum_required(VERSION 3.3)
project(generator)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c99 -lyaml")

set(SOURCE_FILES main.c post.h sds/sdsalloc.h sds/sds.h sundown/autolink.h sundown/buffer.h sundown/houdini.h sundown/html.h sundown/html_blocks.h sundown/markdown.h sundown/stack.h sds/sds.c post.c utils.h utils.c sundown/autolink.c sundown/buffer.c sundown/houdini_href_e.c sundown/houdini_html_e.c sundown/html.c sundown/html_smartypants.c sundown/markdown.c sundown/stack.c)
add_executable(generator ${SOURCE_FILES})