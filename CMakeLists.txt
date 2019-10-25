cmake_minimum_required(VERSION 3.1)
project(glfun)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/)
#set(CMAKE_BUILD_TYPE Debug)
add_subdirectory(libs)
add_subdirectory(src)
