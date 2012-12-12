set(sources
  array.cpp
  bit-vector.cpp
  build.cpp
  divsufsort.cpp
  extract.cpp
  graph.cpp
  rank9b.cpp
  rmq-succinct.cpp
  text.cpp
  utility.cpp
)

add_library(block-graph STATIC ${sources})

target_link_libraries(block-graph rt)
