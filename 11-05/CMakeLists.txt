cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0135 NEW)

project(fibonacci_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
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

enable_testing()

add_executable(fibonacci_tests main.cpp)

target_compile_features(fibonacci_tests PRIVATE cxx_std_23)

if (MSVC)
    target_compile_options(fibonacci_tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(fibonacci_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(fibonacci_tests PRIVATE GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(fibonacci_tests)