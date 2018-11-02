cmake_minimum_required(VERSION 3.2)

option(FGL_SIGNAL_BUILD_TESTS "Include tests in the build tree")

add_subdirectory(fgl_signal)

if(FGL_SIGNAL_BUILD_TESTS)
    add_subdirectory(test)
endif()
