cmake_minimum_required(VERSION 2.8)
project(lab3)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES Lab3.c)
configure_file(Lab3_data.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(lab3 ${SOURCE_FILES})
