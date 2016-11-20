cmake_minimum_required(VERSION 3.6)
project(lab6)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")

set(SOURCE_FILES FP1_JoshBowden_L03.c)
configure_file(default.hex ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(fp1 ${SOURCE_FILES})
