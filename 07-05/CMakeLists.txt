cmake_minimum_required(VERSION 3.16)
project(sort_project CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sort_demo main.cpp)

find_package(GTest REQUIRED)
add_executable(sort_tests main.cpp)
target_compile_definitions(sort_tests PRIVATE SORT_WITH_GTEST)
target_link_libraries(sort_tests PRIVATE GTest::gtest GTest::gtest_main)

find_package(benchmark REQUIRED)
add_executable(sort_bench main.cpp)
target_compile_definitions(sort_bench PRIVATE SORT_WITH_BENCHMARK)
target_link_libraries(sort_bench PRIVATE benchmark::benchmark)