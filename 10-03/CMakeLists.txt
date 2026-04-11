cmake_minimum_required(VERSION 3.14)
project(GameOfLifeMultiArray)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

find_package(Boost REQUIRED)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(run_game_of_life main.cpp)

target_include_directories(run_game_of_life PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(run_game_of_life PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(run_game_of_life)