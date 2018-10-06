cmake_minimum_required(VERSION 3.12)
project(new_huffman)

set(CMAKE_CXX_STANDARD 14)

add_library(huffman
        huffman.cpp
        huffman.h
        )

add_executable(new_huffman
        main.cpp
        )

add_executable(huffman_testing
        gtest/gtest-all.cc
        gtest/gtest.h
        gtest/gtest_main.cc
        testing.cpp
        )

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
endif ()


target_link_libraries(new_huffman huffman)
target_link_libraries(huffman_testing huffman  -lgmp -lgmpxx -lpthread)
