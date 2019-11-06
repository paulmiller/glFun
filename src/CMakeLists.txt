set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wno-unused-parameter -g -O3)

add_subdirectory(math)

set(mesh_obj_parser_input  "${CMAKE_CURRENT_SOURCE_DIR}/mesh_obj_parser.re.cc")
set(mesh_obj_parser_output "${CMAKE_CURRENT_BINARY_DIR}/mesh_obj_parser.cc")

add_custom_command(
  DEPENDS ${mesh_obj_parser_input}
  OUTPUT ${mesh_obj_parser_output}
  COMMAND re2c -W ${mesh_obj_parser_input} -o ${mesh_obj_parser_output}
)

set(common_sources
  bool_voxel_volume.cc
  camera.cc
  camera_control.cc
  color.cc
  csg.cc
  gl_util.cc
  gl_viewport_control.cc
  glfw_window.cc
  image.cc
  image_hdr.cc
  image_png.cc
  labeled_voxel_volume.cc
  memory_usage_linux.cc
  mesh.cc
  mesh_obj.cc
  mesh_obj_parser.cc
  ohno.cc
  ray.cc
  scoped_timer.cc
  util.cc
  voxel_volume.cc
)

set(main_sources
  explore_shapes.cc
  main.cc
)

set(test_sources
  catch_main.cc
  image_test.cc
  util_test.cc
)

include_directories(./)

add_executable(glfun ${main_sources} ${common_sources})
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
