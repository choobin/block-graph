include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../lib")

foreach(bin build extract)
  add_executable(${bin} ${bin}.cpp)
  target_link_libraries(${bin} block-graph)
endforeach()

add_executable(generate-extract-file generate-extract-file.cpp)
