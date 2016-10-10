cmake_minimum_required(VERSION 3.6)
project(lab4)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")

set(SOURCE_FILES Lab4.c)

add_executable(lab4 ${SOURCE_FILES})
