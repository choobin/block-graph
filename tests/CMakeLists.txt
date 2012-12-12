include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../lib")

set(tests
  test-array
  test-bit-vector
  test-extract
  test-extract-extensive
  test-rank
  test-text
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/tests)

foreach(bin ${tests})
  add_executable(${bin} ${bin}.cpp)
  target_link_libraries(${bin} block-graph)
endforeach()
