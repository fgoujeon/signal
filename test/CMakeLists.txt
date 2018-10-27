cmake_minimum_required(VERSION 3.2)

file(GLOB SOURCE_FILES src/*.*)

add_executable(test ${SOURCE_FILES})
