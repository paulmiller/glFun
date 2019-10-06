cmake_minimum_required(VERSION 3.1)
project(glfun)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/)
add_subdirectory(libs)
add_subdirectory(src)
