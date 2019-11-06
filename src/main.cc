#include "camera_control.h"
#include "gl_util.h"
#include "gl_viewport_control.h"
#include "image.h"
#include "image_png.h"
#include "mesh.h"
#include "mesh_obj.h"
#include "ohno.h"
#include "util.h"

#include <cassert>
#include <iostream>
#include <vector>

class Vbo {
public:
  GLuint id;
  GLint size;
};

class Drawable {
public:
  virtual ~Drawable() {}

  virtual void SetUp() = 0;
  virtual void Draw() = 0;
  virtual void TearDown() = 0;

  void SetCameraControl(CameraControl *cc) { camera_control_ = cc; }

protected:
  void GenericDraw() {
    glUseProgram(program_id_);
    GLuint vbo_size = vbos_.size();
    for(GLuint i = 0; i < vbo_size; i++) {
      glEnableVertexAttribArray(i);
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[i].id);
      glVertexAttribPointer(i, vbos_[i].size, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    Matrix4x4f camera_transform = camera_control_->getCam()->getTransform();
    UniformMatrix(mvp_uniform_location_, camera_transform);

    assert(CheckGl());

    glDrawArrays(draw_mode_, 0, draw_count_);

    assert(CheckGl());

    glUseProgram(0);
    for(GLuint i = 0; i < vbo_size; i++) {
      glDisableVertexAttribArray(0);
    }

    assert(CheckGl());
  }

  void GenericTearDown() {
    glDeleteProgram(program_id_);
    for(Vbo &vbo: vbos_)
      glDeleteBuffers(1, &vbo.id);
    vbos_.clear();

    assert(CheckGl());
  }

  GLenum draw_mode_;
  GLsizei draw_count_ = 0;
  GLuint program_id_ = 0;
  GLint mvp_uniform_location_;
  std::vector<Vbo> vbos_;

private:
  CameraControl *camera_control_ = nullptr;
};

class DrawableAxes : public Drawable {
public:
  void SetUp() override {
    std::string obj = readWholeFileOrThrow("res/axes.obj");
    WavFrObj parser;
    parser.ParseFrom(obj);
    TriMesh mesh = parser.GetTriMesh("axes_default");

    GLuint vert_shader_id =
      LoadShader("src/norm_tex_vert.glsl", GL_VERTEX_SHADER);
    GLuint frag_shader_id =
      LoadShader("src/norm_tex_frag.glsl", GL_FRAGMENT_SHADER);
    GLuint program_id = LinkProgram(vert_shader_id, frag_shader_id);
    glDeleteShader(vert_shader_id);
    glDeleteShader(frag_shader_id);

    draw_mode_ = GL_TRIANGLES;
    draw_count_ = mesh.tris.size() * 3;
    program_id_ = program_id;
    mvp_uniform_location_ = glGetUniformLocation(program_id, "mvp");

    vbos_.reserve(3);
    vbos_.push_back({MakeVertexVbo(mesh), 3});
    vbos_.push_back({MakeNormVbo(mesh), 3});
    vbos_.push_back({MakeUvVbo(mesh), 2});

    texture_id_ = MakeTextureFromPng("res/axes.png");
    sampler_uniform_location_ = glGetUniformLocation(program_id, "sampler");

    assert(CheckGl());
  }

  void Draw() override {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glUniform1i(sampler_uniform_location_, 0);

    assert(CheckGl());

    GenericDraw();
  }

  void TearDown() override {
    glDeleteTextures(1, &texture_id_);
    GenericTearDown();
  }

private:
  GLuint texture_id_;
  GLint sampler_uniform_location_;
};

int submain();

const int default_width = 512;
const int default_height = 512;

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
  assert(CheckGl());

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  assert(CheckGl());

  GlViewportControl viewport_control;
  CameraControl camera_control(default_width, default_height);
  GlfwWindow window;
  window.addObserver(&viewport_control);
  window.addObserver(&camera_control);
  window.create(default_width, default_height, "toy");

  assert(CheckGl());

  assert(CheckGl());

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  assert(CheckGl());

  // https://www.opengl.org/wiki/OpenGL_Loading_Library
  glewExperimental = true;
  if(glewInit() != GLEW_OK) {
    std::cout << "glewInit failed" << std::endl;
    return 1;
  }
  #ifndef NDEBUG
  GLenum glew_error =
  #endif
    glGetError();
  assert(glew_error == GL_NO_ERROR || glew_error == GL_INVALID_ENUM);

  GLuint array_id;
  glGenVertexArrays(1, &array_id);
  glBindVertexArray(array_id);

  assert(CheckGl());

  DrawableAxes axes;
  axes.SetCameraControl(&camera_control);
  axes.SetUp();

  assert(CheckGl());

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);

  glClearColor(0.125f, 0.125f, 0.125f, 0.0f);
  glClearDepth(-1.0f);

  assert(CheckGl());

  int64_t frame = 0;
  while(!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    axes.Draw();
    assert(CheckGl());
    glfwSwapBuffers(window);
    glfwPollEvents();
    assert(CheckGl());
    frame++;
  }

  axes.TearDown();

  glDeleteVertexArrays(1, &array_id);

  assert(CheckGl());

  return 0;
}
