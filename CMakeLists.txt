cmake_minimum_required(VERSION 3.27)
project(tomlreduce C)

set(CMAKE_C_STANDARD 99)

# use strict C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

add_library(tomlc99 STATIC dep/tomlc99/toml.c dep/tomlc99/toml.h)

add_executable(tomlreduce main.c tomlprint.h tomlprint.c fmtlogic.h fmtlogic.c filestream.h filestream.c)

target_include_directories(tomlreduce PRIVATE dep)
target_link_libraries(tomlreduce tomlc99)
