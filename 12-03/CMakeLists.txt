cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0135 NEW)

project(task_12_03 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(task_12_03 main.cpp)

if (MSVC)
    target_compile_options(task_12_03 PRIVATE /W4 /permissive-)
else()
    target_compile_options(task_12_03 PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(task_12_03_tests main.cpp)
target_compile_definitions(task_12_03_tests PRIVATE PALINDROME_BUILD_TESTS)
target_link_libraries(task_12_03_tests PRIVATE GTest::gtest GTest::gtest_main)

if (MSVC)
    target_compile_options(task_12_03_tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(task_12_03_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GoogleTest)
gtest_discover_tests(task_12_03_tests)