cmake_minimum_required(VERSION 3.3)
project(generator C)
set(GUI false)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c99")

set(SOURCE_FILES post.h sds/sdsalloc.h sds/sds.h sundown/autolink.h sundown/buffer.h sundown/houdini.h sundown/html.h sundown/html_blocks.h sundown/markdown.h sundown/stack.h sds/sds.c post.c utils.h utils.c sundown/autolink.c sundown/buffer.c sundown/houdini_href_e.c sundown/houdini_html_e.c sundown/html.c sundown/html_smartypants.c sundown/markdown.c sundown/stack.c gen.h gen.c constants.h choose_ui.h)
if(GUI)
    set(SOURCE_FILES ${SOURCE_FILES} gui.c gui.h)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    # Setup CMake to use GTK+, tell the compiler where to look for headers
    # and to the linker where to look for libraries
    include_directories(${GTK3_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})
    # Add other flags to the compiler
    add_definitions(${GTK3_CFLAGS_OTHER})
else()
    set(SOURCE_FILES ${SOURCE_FILES} cli.c cli.h)
endif()
add_executable(generator ${SOURCE_FILES} main.c)
if(GUI)
    target_link_libraries(generator ${GTK3_LIBRARIES})
endif()