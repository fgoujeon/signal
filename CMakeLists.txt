cmake_minimum_required(VERSION 3.2)

option(FGSIG_BUILD_TESTS "Include fgsig tests in the build tree")

add_subdirectory(fgsig)

if(FGSIG_BUILD_TESTS)
    add_subdirectory(test)
endif()
