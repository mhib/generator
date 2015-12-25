cmake_minimum_required(VERSION 3.3)
project(generator)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -lyaml")

set(SOURCE_FILES main.c post.h sds/sdsalloc.h sds/sds.h yaml/yaml.h yaml/yaml_private.h sds/sds.c post.c yaml/api.c yaml/dumper.c/ yaml/emitter.c yaml/loader.c yaml/parser.c yaml/reader.c yaml/scanner.c yaml/writer.c)
add_executable(generator ${SOURCE_FILES})