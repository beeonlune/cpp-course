cmake_minimum_required(VERSION 3.14)
project(Log2Tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(run_log_tests main.cpp)
target_link_libraries(run_log_tests gtest_main)

include(GoogleTest)
gtest_discover_tests(run_log_tests)