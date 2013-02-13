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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

find_package(OpenMP)

if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_BUILD_TYPE "Debug")
  set(CMAKE_VERBOSE_MAKEFILE ON)
else()
  set(CMAKE_BUILD_TYPE "Release")

  # GCC 4.2 introduced the -march=native switch. This will produce code
  # optimized for the host architecture as detected using the cpuid
  # instruction.

  # Too see a list of flags that -march=native activates type:
  # gcc -march=native -Q --help=target

  # This is the easiest method to pull in SSE* and POPCNT compile flags.
  # I.e., I do not want to write my own CMake module and the existing
  # FindSSE modules do not cut it.

  if (CMAKE_COMPILER_IS_GNUCC)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_GREATER 4.2 OR GCC_VERSION VERSION_EQUAL 4.2)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    endif()
  endif()
endif()

add_subdirectory(lib)

add_subdirectory(src)

add_subdirectory(tests)