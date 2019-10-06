set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wno-unused-parameter -g)

add_subdirectory(math)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mesh_obj.cc
  COMMAND re2c -W ${CMAKE_CURRENT_SOURCE_DIR}/mesh_obj.re.cc -o ${CMAKE_CURRENT_BINARY_DIR}/mesh_obj.cc
)

set(common_sources
  camera.cc
  camera_control.cc
  gl_util.cc
  gl_viewport_control.cc
  glfw_window.cc
  image.cc
  image_hdr.cc
  image_png.cc
  mesh.cc
  mesh_obj.cc
  ohno.cc
  util.cc
  vox_vol.cc
)

set(main_sources main.cc)

set(test_sources
  catch_main.cc
  image_test.cc
)

include_directories(./)

add_executable(glfun main.cc ${common_sources})
target_link_libraries(glfun math)
target_link_libraries(glfun xxHash::xxhash)

add_executable(test ${test_sources} ${common_sources})
target_link_libraries(test math)
target_link_libraries(test catch)

set(external_libs
  m
  GL
  GLEW
  glfw
  png
)

foreach(lib ${external_libs})
  find_library(${lib}_lib ${lib})
  target_link_libraries(glfun ${${lib}_lib})
  target_link_libraries(test ${${lib}_lib})
endforeach(lib)
