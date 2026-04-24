cmake_minimum_required(VERSION 3.14)
project(HashFunctionsAnalysis)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)

    if(NOT APPLE)
        add_compile_options(-m32)
        add_link_options(-m32)
    else()
        message(WARNING "macOS does not support the -m32 flag. Skipping 32-bit.")
    endif()
endif()

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(run_hash_analysis main.cpp)
target_link_libraries(run_hash_analysis PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(run_hash_analysis)