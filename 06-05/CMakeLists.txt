cmake_minimum_required(VERSION 3.28)

project(dynamic_libraries_task VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.85 REQUIRED COMPONENTS filesystem system)

add_library(lib_v1 SHARED main.cpp)
target_compile_definitions(lib_v1 PRIVATE BUILD_LIB_V1)
target_compile_options(lib_v1 PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(lib_v1 PROPERTIES PREFIX "")

add_library(lib_v2 SHARED main.cpp)
target_compile_definitions(lib_v2 PRIVATE BUILD_LIB_V2)
target_compile_options(lib_v2 PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(lib_v2 PROPERTIES PREFIX "")

add_executable(main main.cpp)
target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(main PRIVATE ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})

