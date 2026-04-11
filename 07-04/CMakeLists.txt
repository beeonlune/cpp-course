cmake_minimum_required(VERSION 3.16)
project(sort_tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sort_tests main.cpp)
target_compile_definitions(sort_tests PRIVATE SORT_WITH_GTEST)

find_package(GTest REQUIRED)

target_link_libraries(sort_tests PRIVATE GTest::gtest GTest::gtest_main)