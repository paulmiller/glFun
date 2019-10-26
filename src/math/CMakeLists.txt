project(glfun)

set(sources
  matrix.cc
  vector.cc
)

set(test_sources
  matrix_test.cc
  matrix_factories_test.cc
  matrix_vector_product_test.cc
  util_test.cc
  vector_test.cc
  ../catch_main.cc
)

add_library(math ${sources})
target_include_directories(math PUBLIC ./)

add_executable(math_test ${test_sources})
target_include_directories(math_test PRIVATE ./)
# for approx_container.h & catch_main.cc
target_include_directories(math_test PRIVATE ../)
target_link_libraries(math_test math)
target_link_libraries(math_test catch)
