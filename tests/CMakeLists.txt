cmake_minimum_required(VERSION 3.9)
project(tests)
add_executable(tests 
  src/main.c
src/test_manager.c
src/memory/linear_alloc_tests.c)
include_directories(${CMAKE_SOURCE_DIR}/sapfire)
target_link_libraries(tests PUBLIC sapfire)
