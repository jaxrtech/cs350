cmake_minimum_required(VERSION 3.6)
project(lab2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES lab2.c)
add_executable(lab2 ${SOURCE_FILES})