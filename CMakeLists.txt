cmake_minimum_required(VERSION 2.6)

project(block-graph)

add_definitions(
  -Wall
  -Wextra
  -Werror
  -Wshadow
  -Wcast-qual
  -Wcast-align
  -Wwrite-strings
)

find_package(OpenMP)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_BUILD_TYPE "Debug")
  set(CMAKE_VERBOSE_MAKEFILE ON)
  add_subdirectory(tests)
else()
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_subdirectory(lib)

add_subdirectory(src)
