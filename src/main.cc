#include "camera_control.h"
#include "gl_util.h"
#include "gl_viewport_control.h"
#include "half_edge_mesh.h"
#include "image.h"
#include "image_png.h"
#include "mesh.h"
#include "mesh_obj.h"
#include "ohno.h"
#include "ray.h"
#include "scoped_timer.h"
#include "util.h"

#include <cassert>
#include <fstream>
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
  void GenericDrawBegin() {
    glUseProgram(program_id_);
    assert(CheckGl());
    GLuint vbo_size = vbos_.size();
    for(GLuint i = 0; i < vbo_size; i++) {
      glEnableVertexAttribArray(i);
      assert(CheckGl());
      glBindBuffer(GL_ARRAY_BUFFER, vbos_[i].id);
      assert(CheckGl());
      glVertexAttribPointer(i, vbos_[i].size, GL_FLOAT, GL_FALSE, 0, nullptr);
      assert(CheckGl());
    }
    Matrix4x4f camera_transform = camera_control_->getCam()->getTransform();
    UniformMatrix(mvp_uniform_location_, camera_transform);
    assert(CheckGl());
  }

  void GenericDrawEnd() {
    glUseProgram(0);
    assert(CheckGl());
    GLuint vbo_size = vbos_.size();
    for(GLuint i = 0; i < vbo_size; i++) {
      glDisableVertexAttribArray(i);
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

    program_id_ = program_id;
    mvp_uniform_location_ = glGetUniformLocation(program_id, "mvp");

    vbos_.reserve(3);
    vbos_.push_back({MakeVertexVbo(mesh), 3});
    vbos_.push_back({MakeNormVbo(mesh), 3});
    vbos_.push_back({MakeUvVbo(mesh), 2});

    num_verts_ = mesh.tris.size() * 3;
    texture_id_ = MakeTextureFromPng("res/axes.png");
    sampler_uniform_location_ = glGetUniformLocation(program_id, "sampler");

    assert(CheckGl());
  }

  void Draw() override {
    GenericDrawBegin();

    glActiveTexture(GL_TEXTURE0);
    assert(CheckGl());
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    assert(CheckGl());
    glUniform1i(sampler_uniform_location_, 0);
    assert(CheckGl());

    glDrawArrays(GL_TRIANGLES, 0, num_verts_);
    assert(CheckGl());

    GenericDrawEnd();
  }

  void TearDown() override {
    glDeleteTextures(1, &texture_id_);
    assert(CheckGl());
    GenericTearDown();
  }

private:
  GLsizei num_verts_;
  GLuint texture_id_;
  GLint sampler_uniform_location_;
};

class DrawableLinesBase : public Drawable {
public:
  void SetUp() override {
    GLuint vert_shader_id =
      LoadShader("src/lines_vert.glsl", GL_VERTEX_SHADER);
    GLuint frag_shader_id =
      LoadShader("src/lines_frag.glsl", GL_FRAGMENT_SHADER);
    GLuint program_id = LinkProgram(vert_shader_id, frag_shader_id);
    glDeleteShader(vert_shader_id);
    glDeleteShader(frag_shader_id);
    program_id_ = program_id;
    mvp_uniform_location_ = glGetUniformLocation(program_id, "mvp");
    SetUpVbo();
    assert(CheckGl());
  }

  void Draw() override {
    GenericDrawBegin();
    glDrawArrays(GL_LINES, 0, num_verts_);
    assert(CheckGl());
    GenericDrawEnd();
  }

  void TearDown() override {
    GenericTearDown();
  }

protected:
  virtual void SetUpVbo() = 0;

  GLsizei num_verts_ = 0;
};

class DrawableLines : public DrawableLinesBase {
public:
  DrawableLines(std::vector<std::pair<Vector3f,Vector3f>> lines) :
    lines_(std::move(lines)) {}

protected:
  void SetUpVbo() override {
    vbos_.reserve(1);
    vbos_.push_back({MakeLinesVbo(lines_), 3});
    num_verts_ = lines_.size() * 2;
  }

private:
  std::vector<std::pair<Vector3f,Vector3f>> lines_;
};

class DrawableRays : public DrawableLinesBase {
public:
  DrawableRays(std::vector<Ray> rays) : rays_(std::move(rays)) {}

protected:
  void SetUpVbo() override {
    vbos_.reserve(1);
    vbos_.push_back({MakeRaysVbo(rays_), 3});
    num_verts_ = rays_.size() * 2;
  }

private:
  GLsizei num_verts_;
  std::vector<Ray> rays_;
};

DrawableLines MakeDrawableHalfEdges(
  const HalfEdgeMesh &mesh,
  const std::unordered_set<HalfEdgeMesh::HalfEdgeIndex> edge_indices
) {
  std::vector<std::pair<Vector3f,Vector3f>> lines;
  for(const HalfEdgeMesh::HalfEdgeIndex &edge_index: edge_indices) {
    const HalfEdgeMesh::HalfEdge *edge = &mesh[edge_index];
    const Vector3d &start = *(edge->twin_edge->vertex->position);
    const Vector3d &end = *(edge->vertex->position);
    lines.push_back(std::make_pair(static_cast<Vector3f>(start),
                                   static_cast<Vector3f>(end)));
  }
  return DrawableLines(std::move(lines));
}

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

  //HalfEdgeMesh mesh = MakeAlignedCells();
  HalfEdgeMesh mesh = MakeGeoSphere(1);

  {
    /*
    PrintingScopedTimer timer("mesh");

    Vector3d bisect_normals[] = {
      Vector3d{1,1,0}, Vector3d{1,-1, 0},
      Vector3d{1,0,1}, Vector3d{1, 0,-1},
      Vector3d{0,1,1}, Vector3d{0, 1,-1}};
    for(const Vector3d &normal: bisect_normals)
      mesh.LoopCut(mesh.Bisect(normal));
    */

    /*
    auto bisect_edge_indices = mesh.Bisect(Vector3d{1,1,0});
    DrawableLines lines =
      MakeDrawableHalfEdges(mesh, std::move(bisect_edge_indices));
    lines.SetCameraControl(&camera_control);
    lines.SetUp();
    */

    WavFrObj obj = mesh.MakeWavFrObj();
    std::string text = obj.Export();
    std::ofstream out("out.obj", std::ofstream::out);
    out << text;
    out.close();
  }

  /*
  assert(CheckGl());

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);

  glClearColor(0.125f, 0.125f, 0.125f, 0.0f);
  glClearDepth(-1.0f);

  assert(CheckGl());

  int64_t frame = 0;
  while(!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    assert(CheckGl());
    axes.Draw();
    assert(CheckGl());
    / *
    lines.Draw();
    assert(CheckGl());
    * /
    glfwSwapBuffers(window);
    glfwPollEvents();
    assert(CheckGl());
    frame++;
  }

  axes.TearDown();

  glDeleteVertexArrays(1, &array_id);

  assert(CheckGl());
  */

  return 0;
}
