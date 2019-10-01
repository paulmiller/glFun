#include "camera_control.h"
#include "gl_util.h"
#include "gl_viewport_control.h"
#include "image.h"
#include "image_hdr.h"
#include "image_png.h"
#include "mesh.h"
#include "mesh_obj.h"
#include "ohno.h"
#include "util.h"
#include "vox_vol.h"

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

int submain();

const int default_width = 512;
const int default_height = 512;

/*
enum KeyCommand {
  FORWARD,
  BACKWARD,
  STRAFE_LEFT,
  STRAFE_RIGHT,
  TURN_LEFT,
  TURN_RIGHT,
  UP,
  DOWN,
  ESCAPE
};

class KeyInput {
public:
  const KeyCommand command;
  const int code;
  bool pressed;

  KeyInput(KeyCommand command, int code) :
    command(command), code(code), pressed(false) {};
};

KeyInput controls[] = {
  {FORWARD,      GLFW_KEY_COMMA},
  {BACKWARD,     GLFW_KEY_O},
  {STRAFE_LEFT,  GLFW_KEY_A},
  {STRAFE_RIGHT, GLFW_KEY_E},
  {TURN_LEFT,    GLFW_KEY_APOSTROPHE},
  {TURN_RIGHT,   GLFW_KEY_PERIOD},
  {UP,           GLFW_KEY_SPACE},
  {DOWN,         GLFW_KEY_SEMICOLON},
  {ESCAPE,       GLFW_KEY_ESCAPE}
};

const int control_num = sizeof(controls) / sizeof(KeyInput);

bool getCommandState(KeyCommand command) {
  for(int i = 0; i < control_num; i++) {
    if(command == controls[i].command)
      return controls[i].pressed;
  }
  assert(0);
  return false;
}

void onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
  for(int i = 0; i < control_num; i++) {
    if(key == controls[i].code) {
      if(action == GLFW_PRESS)
        controls[i].pressed = true;
      else if(action == GLFW_RELEASE)
        controls[i].pressed = false;
      break;
    }
  }
}

void onMouseButton(GLFWwindow *window, int button, int action, int mods) {
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    std::cout << "click x=" << xpos << " y=" << ypos << std::endl;
    Vector3f near, far;
    camera_ptr->castPixel(int(xpos), int(ypos), near, far);
    std::cout << "cast near=" << near << " far=" << far << std::endl;
    std::cout << "hit=" << widget_ptr->intersects(near, far - near) << std::endl;
  }
}
*/

int main() {
  if(!glfwInit()) {
    std::cout << "glfwInit failed" << std::endl;
    return 1;
  }

  int ret;
  try {
    ret = submain();
  } catch(OhNo ohno) {
    std::cout << ohno << std::endl;
    ret = 1;
  }

  glfwTerminate();
  return ret;
}

#include <cmath>

int submain() {
  assert(checkGL());

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  assert(checkGL());

  GlViewportControl viewport_control;
  CameraControl camera_control(default_width, default_height);
  GlfwWindow window;
  window.addObserver(&viewport_control);
  window.addObserver(&camera_control);
  window.create(default_width, default_height, "toy");

  assert(checkGL());

  assert(checkGL());

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  assert(checkGL());

  // https://www.opengl.org/wiki/OpenGL_Loading_Library
  glewExperimental = true;
  if(glewInit() != GLEW_OK) {
    std::cout << "glewInit failed" << std::endl;
    return 1;
  }
  GLenum glew_error = glGetError();
  assert(glew_error == GL_NO_ERROR || glew_error == GL_INVALID_ENUM);

  GLuint color = pngTex("res/axes.png");

  assert(checkGL());

  GLuint array_id;
  glGenVertexArrays(1, &array_id);
  glBindVertexArray(array_id);

  std::string axes_str = readWholeFileOrThrow("res/axes.obj");
  WavFrObj parser;
  parser.parseFrom(axes_str);
  TriMesh axes_mesh = parser.getTriMesh("axes_default");
  GLuint axes_vert_buffer_id = vertVBO(axes_mesh);
  GLuint axes_uv_buffer_id = uvVBO(axes_mesh);
  GLuint axes_norm_buffer_id = normVBO(axes_mesh);

  assert(checkGL());

  int vox_vol_size = 200;
  VoxVol vox_vol(vox_vol_size, vox_vol_size, vox_vol_size); 
  for(int z = 0; z < vox_vol_size; z++) {
    for(int y = 0; y < vox_vol_size; y++) {
      for(int x = 0; x < vox_vol_size; x++) {
        Vector3f v = vox_vol.centerOf(x, y, z);
        if(pow(v.x,12) + pow(v.y,12) + pow(v.z,12) < 1.0f) {
          vox_vol.at(x, y, z) = true;
        }
      }
    }
  }
  auto t1 = std::chrono::steady_clock::now();
  TriMesh vox_vol_mesh = vox_vol.createBlockMesh();
  auto t2 = std::chrono::steady_clock::now();
  {
    auto seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(t2-t1).count();
    size_t verts_size = vox_vol_mesh.verts.size();
    size_t tris_size = vox_vol_mesh.tris.size();
    std::cout << "vox mesh: "
      << verts_size << " verts ("
      << (verts_size * sizeof(Vector3f) / 1024) << " KiB), "
      << tris_size << " tris, ("
      << (tris_size * sizeof(Tri) / 1024) << " KiB), "
      << seconds << " seconds\n";
  }

  GLuint vox_vol_vert_buffer_id = vertVBO(vox_vol_mesh);
  GLuint vox_vol_norm_buffer_id = normVBO(vox_vol_mesh);

  assert(checkGL());

  GLuint norm_vert_shader_id =
    loadShader("src/norm_vert.glsl", GL_VERTEX_SHADER);
  GLuint norm_frag_shader_id =
    loadShader("src/norm_frag.glsl", GL_FRAGMENT_SHADER);
  GLuint norm_program_id =
    linkProgram(norm_vert_shader_id, norm_frag_shader_id);
  glDeleteShader(norm_vert_shader_id);
  glDeleteShader(norm_frag_shader_id);

  assert(checkGL());

  GLuint norm_program_mvp_id = glGetUniformLocation(norm_program_id, "mvp");

  assert(checkGL());

  GLuint norm_tex_vert_shader_id =
    loadShader("src/norm_tex_vert.glsl", GL_VERTEX_SHADER);
  GLuint norm_tex_frag_shader_id =
    loadShader("src/norm_tex_frag.glsl", GL_FRAGMENT_SHADER);
  GLuint norm_tex_program_id =
    linkProgram(norm_tex_vert_shader_id, norm_tex_frag_shader_id);
  glDeleteShader(norm_tex_vert_shader_id);
  glDeleteShader(norm_tex_frag_shader_id);

  assert(checkGL());

  GLuint norm_tex_program_mvp_id =
    glGetUniformLocation(norm_tex_program_id, "mvp");
  GLuint norm_tex_program_sampler_id =
    glGetUniformLocation(norm_program_id, "sampler");

  assert(checkGL());

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glClearDepth(-1.0f);

  int64_t frame = 0;
  while(!glfwWindowShouldClose(window)) {
    /*
    bool going_forward  = getCommandState(FORWARD);
    bool going_backward = getCommandState(BACKWARD);
    bool going_left     = getCommandState(STRAFE_LEFT);
    bool going_right    = getCommandState(STRAFE_RIGHT);
    bool going_up       = getCommandState(UP);
    bool going_down     = getCommandState(DOWN);

    Vector3f move = Zero_Vector3f;

    if(going_forward && !going_backward)
      move += Vector3f(0, 0, -1);
    else if(going_backward && !going_forward)
      move += Vector3f(0, 0, 1);

    if(going_left && !going_right)
      move += Vector3f(-1, 0, 0);
    else if(going_right && !going_left)
      move += Vector3f(1, 0, 0);

    if(going_up && !going_down)
      move += Vector3f(0, 1, 0);
    else if(going_down && !going_up)
      move += Vector3f(0, -1, 0);

    if(move != Zero_Vector3f) {
      move = move.unit() * 0.05;
      camera_obj.moveLocal(move);
    }

    bool turning_left  = getCommandState(TURN_LEFT);
    bool turning_right = getCommandState(TURN_RIGHT);

    if(turning_left && !turning_right)
      camera_obj.rot *= Quat::rotation(UnitY_Vector3f, 0.03);
    else if(turning_right && !turning_left)
      camera_obj.rot *= Quat::rotation(UnitY_Vector3f, -0.03);

    Vector3f forward = (Matrix4x4f::rotation(camera_obj.rot) * -Vector4f::UNIT_Z).dropW();
    camera.look(camera_obj.pos, forward, UnitY_Vector3f);
    */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw axes

    glUseProgram(norm_tex_program_id);

    UniformMatrix(norm_tex_program_mvp_id,
                  camera_control.getCam()->getTransform());

    assert(checkGL());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color);
    glUniform1i(norm_tex_program_sampler_id, 0);

    assert(checkGL());

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, axes_vert_buffer_id);
    glVertexAttribPointer(
      0,        // index (attribute)
      3,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0,        // stride
      (void*)0  // pointer (buffer offset)
    );

    assert(checkGL());

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, axes_norm_buffer_id);
    glVertexAttribPointer(
      1,        // index (attribute)
      3,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0,        // stride
      (void*)0  // pointer (buffer offset)
    );

    assert(checkGL());

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, axes_uv_buffer_id);
    glVertexAttribPointer(
      2,        // index (attribute)
      2,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0,        // stride
      (void*)0  // pointer (buffer offset)
    );

    assert(checkGL());

    glDrawArrays(GL_TRIANGLES, 0, axes_mesh.tris.size() * 3);

    assert(checkGL());

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    assert(checkGL());

    // draw vox_vol

    glUseProgram(norm_program_id);

    UniformMatrix(norm_program_mvp_id, camera_control.getCam()->getTransform());

    assert(checkGL());

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vox_vol_vert_buffer_id);
    glVertexAttribPointer(
      0,        // index (attribute)
      3,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0,        // stride
      (void*)0  // pointer (buffer offset)
    );

    assert(checkGL());

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vox_vol_norm_buffer_id);
    glVertexAttribPointer(
      1,        // index (attribute)
      3,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0,        // stride
      (void*)0  // pointer (buffer offset)
    );

    assert(checkGL());

    glDrawArrays(GL_TRIANGLES, 0, vox_vol_mesh.tris.size() * 3);

    assert(checkGL());

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    assert(checkGL());

    // done drawing

    glfwSwapBuffers(window);
    glfwPollEvents();
    assert(checkGL());
    frame++;
  }

  glDeleteProgram(norm_program_id);
  glDeleteBuffers(1, &vox_vol_vert_buffer_id);
  glDeleteBuffers(1, &vox_vol_norm_buffer_id);

  glDeleteProgram(norm_tex_program_id);
  glDeleteBuffers(1, &axes_vert_buffer_id);
  glDeleteBuffers(1, &axes_uv_buffer_id);
  glDeleteBuffers(1, &axes_norm_buffer_id);
  glDeleteTextures(1, &color);

  glDeleteVertexArrays(1, &array_id);

  assert(checkGL());

  return 0;
}
