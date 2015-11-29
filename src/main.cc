#include "bytes.h"
#include "camera.h"
#include "image.h"
#include "image_png.h"
#include "mesh.h"
#include "ohno.h"
#include "util.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

int submain();
void glSetup();
bool checkGL();
void testCube();
void testCloud();
void testAxes();

const int defaultWidth = 512;
const int defaultHeight = 512;

Camera *cameraPtr;

void framebufferSize(GLFWwindow *window, int width, int height) {
  assert(cameraPtr);
  float aspect = float(width) / float(height);
  float horizFOV = PI/2;
  cameraPtr->setResolution(width, height);
  cameraPtr->setFrustum(0.1f, 100.0f, horizFOV, aspect);
  glViewport(0, 0, width, height);
}

/*
void mouseButton(GLFWwindow *window, int button, int action, int mods) {
  assert(cameraPtr);
}
*/

/*
void cursorPos(GLFWwindow* window, double xpos, double ypos) {
  assert(cameraPtr);
}
*/

GLuint loadShader(const char *sourceName, GLenum shaderType) {
  assert(shaderType == GL_VERTEX_SHADER || shaderType == GL_FRAGMENT_SHADER);

  GLuint id = glCreateShader(shaderType);
  Bytes source = Bytes::loadFile(sourceName);
  const char *ptr = source.get();
  COMPILE_ASSERT(sizeof(char) == sizeof(GLchar));
  glShaderSource(id, 1, &ptr, nullptr);
  glCompileShader(id);

  GLint result = GL_FALSE;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE) {
    GLint logLength;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
    Bytes log(logLength);
    glGetShaderInfoLog(id, logLength, nullptr, log.get());
    std::cout << "shader compile failed:\n" << log.get();
  }

  assert(checkGL());

  return id;
}

GLuint linkProgram(GLuint vertShaderId, GLuint fragShaderId) {
  GLuint programId = glCreateProgram();
  glAttachShader(programId, vertShaderId);
  glAttachShader(programId, fragShaderId);
  glLinkProgram(programId);

  GLint result = GL_FALSE;
  glGetProgramiv(programId, GL_LINK_STATUS, &result);
  if(result == GL_FALSE) {
    GLint logLength;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
    Bytes log(logLength);
    glGetProgramInfoLog(programId, logLength, nullptr, log.get());
    std::cout << "program link failed:\n" << log.get();
  }

  assert(checkGL());

  return programId;
}

GLuint vertVBO(const Mesh &m) {
  // 3 verts per tri, 3 floats per vert
  int floatNum = m.mTris.size() * 3 * 3;
  GLfloat *vbo = new GLfloat[floatNum];

  int b = 0;
  for(auto it = m.mTris.begin(); it != m.mTris.end(); it++) {
    for(int i = 0; i < 3; i++) {
      const Vec3 &v = m.mVerts[it->vertIdxs[i]];
      vbo[b++] = v.x;
      vbo[b++] = v.y;
      vbo[b++] = v.z;
    }
  }

  GLuint vertBufferId;
  glGenBuffers(1, &vertBufferId);
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferId);
  glBufferData(GL_ARRAY_BUFFER, floatNum * sizeof(GLfloat), vbo,
    GL_STATIC_DRAW);

  delete[] vbo;
  return vertBufferId;
}

GLuint uvVBO(const Mesh &m) {
  // 3 verts per tri, 2 floats per vert
  int floatNum = m.mTris.size() * 3 * 2;
  GLfloat *vbo = new GLfloat[floatNum];

  int b = 0;
  for(auto it = m.mTris.begin(); it != m.mTris.end(); it++) {
    for(int i = 0; i < 3; i++) {
      const UVCoord &v = m.mUVs[it->uvIdxs[i]];
      vbo[b++] = v.u;
      vbo[b++] = v.v;
    }
  }

  for(int i = 0; i < 3 * 2; i++) {
    std::cout << vbo[i] << ' ';
  }
  std::cout << std::endl;

  GLuint vertBufferId;
  glGenBuffers(1, &vertBufferId);
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferId);
  glBufferData(GL_ARRAY_BUFFER, floatNum * sizeof(GLfloat), vbo,
    GL_STATIC_DRAW);

  delete[] vbo;
  return vertBufferId;
}

// https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.9 pg 226
GLuint texGlInternalFmt(Pixel::Type t) {
  switch(t) {
    case Pixel::NONE:   assert(0); return 0;

    case Pixel::V8:     return GL_R8;
    case Pixel::V16:    return GL_R16;
    case Pixel::VA8:    assert(0); return 0;
    case Pixel::VA16:   assert(0); return 0;
    case Pixel::RGB8:   return GL_RGBA8;
    case Pixel::RGB16:  return GL_RGBA16;
    case Pixel::RGBA8:  return GL_RGBA8;
    case Pixel::RGBA16: return GL_RGBA16;

    default:
      assert(0);
      return 0;
  }
}

// https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.3 pg 183
GLuint texGlFmt(Pixel::Type t) {
  switch(t) {
    case Pixel::NONE:
      assert(0);
      return 0;

    case Pixel::V8:
    case Pixel::V16:
      return GL_RED;
    case Pixel::VA8:
    case Pixel::VA16:
      assert(0);
      return 0;
    case Pixel::RGB8:
    case Pixel::RGB16:
      return GL_RGB;
    case Pixel::RGBA8:
    case Pixel::RGBA16:
      return GL_RGBA;

    default:
      assert(0);
      return 0;
  }
}

// https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.2 pg 182
GLuint texGlType(Pixel::Type t) {
  switch(t) {
    case Pixel::NONE:
      assert(0);
      return 0;

    case Pixel::V8:
    case Pixel::VA8:
    case Pixel::RGB8:
    case Pixel::RGBA8:
      return GL_UNSIGNED_BYTE;
    case Pixel::V16:
    case Pixel::VA16:
    case Pixel::RGB16:
    case Pixel::RGBA16:
      return GL_UNSIGNED_SHORT;

    default:
      assert(0);
      return 0;
  }
}

GLuint pngTex(const char *pngName) {
  std::ifstream input(pngName, std::ifstream::binary);
  Image texImg = readPng(input);
  Pixel::Type type = texImg.type();
  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(
    GL_TEXTURE_2D,          // target
    0,                      // level
    texGlInternalFmt(type), // internalFormat
    texImg.width(),         // width
    texImg.height(),        // height
    0,                      // border
    texGlFmt(type),         // format
    texGlType(type),        // type
    texImg.data()           // data
  );

  /*
  char *d = (char*) texImg.data();
  for(int i = 0; i < 9; i++) {
    std::cout << (unsigned) d[i] << ' ';
  }
  std::cout << std::endl;

  Image check(1024, 1024, Pixel::RGB8);
  glGetTextureImage(texId, 0, GL_RGB, GL_UNSIGNED_BYTE, 1024*1024*3, check.data());
  d = (char*) check.data();
  for(int i = 0; i < 9; i++) {
    std::cout << (unsigned) d[i] << ' ';
  }
  std::cout << std::endl;
  */

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  return texId;
}

int main() {
  try {
    return submain();
  } catch(OhNo ohno) {
    std::cout << ohno << std::endl;
    return 1;
  }
}

#include <cmath>

int submain() {
  if(!glfwInit()) {
    std::cout << "glfwInit failed" << std::endl;
    return 1;
  }

  assert(checkGL());

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  assert(checkGL());

  GLFWwindow* window = glfwCreateWindow(defaultWidth, defaultHeight,
      "toy", NULL, NULL);
  if(!window) {
    std::cout << "glfwCreateWindow failed" << std::endl;
    glfwTerminate();
    return 1;
  }

  assert(checkGL());

  Camera camera;
  cameraPtr = &camera;
  framebufferSize(window, defaultWidth, defaultHeight);
  glfwSetFramebufferSizeCallback(window, framebufferSize);
  //glfwSetMouseButtonCallback(window, mouseButton);
  //glfwSetCursorPosCallback(window, cursorPos);

  assert(checkGL());

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  assert(checkGL());

  // https://www.opengl.org/wiki/OpenGL_Loading_Library
  glewExperimental = true;
  if(glewInit() != GLEW_OK) {
    std::cout << "glewInit failed" << std::endl;
    glfwTerminate();
    return 1;
  }
  GLenum glewError = glGetError();
  assert(glewError == GL_NO_ERROR || glewError == GL_INVALID_ENUM);

  GLuint purple = pngTex("res/uv.png");

  assert(checkGL());

  assert(checkGL());

  GLuint arrayId;
  glGenVertexArrays(1, &arrayId);
  glBindVertexArray(arrayId);

  std::ifstream widgetInput("res/suzanne.obj", std::ifstream::in);
  std::vector<Mesh> widgets = Mesh::parseObj(widgetInput);
  GLuint vertBufferId = vertVBO(widgets[0]);
  GLuint uvBufferId = uvVBO(widgets[0]);

  assert(checkGL());

  GLuint vertShaderId = loadShader("src/vert.glsl", GL_VERTEX_SHADER);
  GLuint fragShaderId = loadShader("src/frag.glsl", GL_FRAGMENT_SHADER);
  GLuint programId = linkProgram(vertShaderId, fragShaderId);
  glDeleteShader(vertShaderId);
  glDeleteShader(fragShaderId);

  assert(checkGL());

  GLuint MatrixID = glGetUniformLocation(programId, "MVP");
  GLuint samplerId = glGetUniformLocation(programId, "myTextureSampler");

  assert(checkGL());

  //glDisable(GL_CULL_FACE);
  //glEnable(GL_RESCALE_NORMAL);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glClearDepth(-1.0f);

  int64_t frame = 0;
  while(!glfwWindowShouldClose(window)) {
    double t = frame / 100.0;
    camera.lookAt(Vec3(2*sin(t), 1, 2*cos(t)), Vec3::ZERO, Vec3::UNIT_Y);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, camera.getTransform().data());

    assert(checkGL());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, purple);
    glUniform1i(samplerId, 0);

    assert(checkGL());

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertBufferId);
    glVertexAttribPointer(
      0,                  // attribute 0. No particular reason for 0,
                          // but must match the layout in the shader.
      3,                  // size
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
    );

    assert(checkGL());

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
    glVertexAttribPointer(
      1,                  // attribute 0. No particular reason for 0,
                          // but must match the layout in the shader.
      2,                  // size
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
    );

    assert(checkGL());

    glDrawArrays(GL_TRIANGLES, 0, widgets[0].mTris.size() * 3);

    assert(checkGL());

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
    assert(checkGL());
    frame++;
  }

  glDeleteProgram(programId);
  glDeleteBuffers(1, &uvBufferId);
  glDeleteBuffers(1, &vertBufferId);
  glDeleteVertexArrays(1, &arrayId);
  glDeleteTextures(1, &purple);

  assert(checkGL());

  glfwTerminate();
  return 0;
}

void glSetup() {
  /*
	static const float lightAmbi[] = {0.1f, 0.1f, 0.1f, 1.0f};
	static const float lightDiff[] = {0.6f, 0.6f, 0.6f, 1.0f};
	static const float lightSpec[] = {0.0f, 0.0f, 0.0f, 1.0f};
	static const float lightPos[] = {0.0f, 0.0f,2.0f, 0.0f};
  */

  //glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);
  //glEnable(GL_RESCALE_NORMAL);

  /*
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbi);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  */

  //glPointSize(4);

  assert(checkGL());
}

bool checkGL() {
  bool noErrors = true;
  while(noErrors) {
    switch(glGetError()) {
      case GL_NO_ERROR:
        return noErrors;
      case GL_INVALID_ENUM:
        std::cout << "GL_INVALID_ENUM" << std::endl;
        noErrors = false;
        break;
      case GL_INVALID_VALUE:
        std::cout << "GL_INVALID_VALUE" << std::endl;
        noErrors = false;
        break;
      case GL_INVALID_OPERATION:
        std::cout << "GL_INVALID_OPERATION" << std::endl;
        noErrors = false;
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
        noErrors = false;
        break;
      case GL_OUT_OF_MEMORY:
        std::cout << "GL_OUT_OF_MEMORY" << std::endl;
        noErrors = false;
        break;
      case GL_STACK_UNDERFLOW:
        std::cout << "GL_STACK_UNDERFLOW" << std::endl;
        noErrors = false;
        break;
      case GL_STACK_OVERFLOW:
        std::cout << "GL_STACK_OVERFLOW" << std::endl;
        noErrors = false;
        break;
      default:
        std::cout << "unknown gl error" << std::endl;
        noErrors = false;
    }
  }
  return noErrors;
}

void testCube() {
  static const float red[]   = {0.8f, 0.1f, 0.1f, 1.0f};
  static const float green[] = {0.2f, 0.8f, 0.2f, 1.0f};
  static const float blue[]  = {0.1f, 0.4f, 1.0f, 1.0f};
  static const float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
	static const float shine[] = {10.0f};

  glMaterialfv(GL_FRONT, GL_SHININESS, shine);
  glMaterialfv(GL_FRONT, GL_SPECULAR, white);

  glBegin(GL_QUADS);
    glMaterialfv(GL_FRONT, GL_AMBIENT, red);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, red);

    glNormal3d(-1.0, 0.0, 0.0);
    glVertex3d(-1.0,-1.0,-1.0);
    glVertex3d(-1.0,-1.0, 1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(-1.0, 1.0,-1.0);

    glNormal3d( 1.0, 0.0, 0.0);
    glVertex3d( 1.0,-1.0,-1.0);
    glVertex3d( 1.0, 1.0,-1.0);
    glVertex3d( 1.0, 1.0, 1.0);
    glVertex3d( 1.0,-1.0, 1.0);

    glMaterialfv(GL_FRONT, GL_AMBIENT, green);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, green);

    glNormal3d( 0.0,-1.0, 0.0);
    glVertex3d(-1.0,-1.0,-1.0);
    glVertex3d( 1.0,-1.0,-1.0);
    glVertex3d( 1.0,-1.0, 1.0);
    glVertex3d(-1.0,-1.0, 1.0);

    glNormal3d( 0.0, 1.0, 0.0);
    glVertex3d(-1.0, 1.0,-1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d( 1.0, 1.0, 1.0);
    glVertex3d( 1.0, 1.0,-1.0);

    glMaterialfv(GL_FRONT, GL_AMBIENT, blue);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);

    glNormal3d( 0.0, 0.0,-1.0);
    glVertex3d(-1.0,-1.0,-1.0);
    glVertex3d(-1.0, 1.0,-1.0);
    glVertex3d( 1.0, 1.0,-1.0);
    glVertex3d( 1.0,-1.0,-1.0);

    glNormal3d( 0.0, 0.0, 1.0);
    glVertex3d(-1.0,-1.0, 1.0);
    glVertex3d( 1.0,-1.0, 1.0);
    glVertex3d( 1.0, 1.0, 1.0);
    glVertex3d(-1.0, 1.0, 1.0);
  glEnd();
}

void testCloud() {
  glBegin(GL_POINTS);
  for(int x = -10; x <= 10; x++) {
    double xd = x / 10.0;
    for(int y = -10; y <= 10; y++) {
      double yd = y / 10.0;
      for(int z = -10; z <= 10; z++) {
        double zd = z / 10.0;
        glColor3d(xd, yd, zd);
        glVertex3d(xd, yd, zd);
      }
    }
  }
  glEnd();
}

void testAxes() {
  glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
    glColor3d(1.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(1.0, 0.0, 0.0);

    glColor3d(0.0, 1.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 1.0, 0.0);

    glColor3d(0.0, 0.0, 1.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, 1.0);
  glEnd();
  glEnable(GL_LIGHTING);
}

/*
const char * tests[] = {
  "basi0g01.png",
  "basi0g02.png",
  "basi0g04.png",
  "basi0g08.png",
  "basi0g16.png",
  "basi2c08.png",
  "basi2c16.png",
  "basi3p01.png",
  "basi3p02.png",
  "basi3p04.png",
  "basi3p08.png",
  "basi4a08.png",
  "basi4a16.png",
  "basi6a08.png",
  "basi6a16.png",
  "basn0g01.png",
  "basn0g02.png",
  "basn0g04.png",
  "basn0g08.png",
  "basn0g16.png",
  "basn2c08.png",
  "basn2c16.png",
  "basn3p01.png",
  "basn3p02.png",
  "basn3p04.png",
  "basn3p08.png",
  "basn4a08.png",
  "basn4a16.png",
  "basn6a08.png",
  "basn6a16.png",
  "bgai4a08.png",
  "bgai4a16.png",
  "bgan6a08.png",
  "bgan6a16.png",
  "bgbn4a08.png",
  "bggn4a16.png",
  "bgwn6a08.png",
  "bgyn6a16.png",
  "ccwn2c08.png",
  "ccwn3p08.png",
  "cdfn2c08.png",
  "cdhn2c08.png",
  "cdsn2c08.png",
  "cdun2c08.png",
  "ch1n3p04.png",
  "ch2n3p08.png",
  "cm0n0g04.png",
  "cm7n0g04.png",
  "cm9n0g04.png",
  "cs3n2c16.png",
  "cs3n3p08.png",
  "cs5n2c08.png",
  "cs5n3p08.png",
  "cs8n2c08.png",
  "cs8n3p08.png",
  "ct0n0g04.png",
  "ct1n0g04.png",
  "cten0g04.png",
  "ctfn0g04.png",
  "ctgn0g04.png",
  "cthn0g04.png",
  "ctjn0g04.png",
  "ctzn0g04.png",
  "f00n0g08.png",
  "f00n2c08.png",
  "f01n0g08.png",
  "f01n2c08.png",
  "f02n0g08.png",
  "f02n2c08.png",
  "f03n0g08.png",
  "f03n2c08.png",
  "f04n0g08.png",
  "f04n2c08.png",
  "f99n0g04.png",
  "g03n0g16.png",
  "g03n2c08.png",
  "g03n3p04.png",
  "g04n0g16.png",
  "g04n2c08.png",
  "g04n3p04.png",
  "g05n0g16.png",
  "g05n2c08.png",
  "g05n3p04.png",
  "g07n0g16.png",
  "g07n2c08.png",
  "g07n3p04.png",
  "g10n0g16.png",
  "g10n2c08.png",
  "g10n3p04.png",
  "g25n0g16.png",
  "g25n2c08.png",
  "g25n3p04.png",
  "oi1n0g16.png",
  "oi1n2c16.png",
  "oi2n0g16.png",
  "oi2n2c16.png",
  "oi4n0g16.png",
  "oi4n2c16.png",
  "oi9n0g16.png",
  "oi9n2c16.png",
  "PngSuite.png",
  "pp0n2c16.png",
  "pp0n6a08.png",
  "ps1n0g08.png",
  "ps1n2c16.png",
  "ps2n0g08.png",
  "ps2n2c16.png",
  "s01i3p01.png",
  "s01n3p01.png",
  "s02i3p01.png",
  "s02n3p01.png",
  "s03i3p01.png",
  "s03n3p01.png",
  "s04i3p01.png",
  "s04n3p01.png",
  "s05i3p02.png",
  "s05n3p02.png",
  "s06i3p02.png",
  "s06n3p02.png",
  "s07i3p02.png",
  "s07n3p02.png",
  "s08i3p02.png",
  "s08n3p02.png",
  "s09i3p02.png",
  "s09n3p02.png",
  "s32i3p04.png",
  "s32n3p04.png",
  "s33i3p04.png",
  "s33n3p04.png",
  "s34i3p04.png",
  "s34n3p04.png",
  "s35i3p04.png",
  "s35n3p04.png",
  "s36i3p04.png",
  "s36n3p04.png",
  "s37i3p04.png",
  "s37n3p04.png",
  "s38i3p04.png",
  "s38n3p04.png",
  "s39i3p04.png",
  "s39n3p04.png",
  "s40i3p04.png",
  "s40n3p04.png",
  "tbbn0g04.png",
  "tbbn2c16.png",
  "tbbn3p08.png",
  "tbgn2c16.png",
  "tbgn3p08.png",
  "tbrn2c08.png",
  "tbwn0g16.png",
  "tbwn3p08.png",
  "tbyn3p08.png",
  "tm3n3p02.png",
  "tp0n0g08.png",
  "tp0n2c08.png",
  "tp0n3p08.png",
  "tp1n3p08.png",
  "xc1n0g08.png",
  "xc9n2c08.png",
  "xcrn0g04.png",
  "xcsn0g01.png",
  "xd0n2c08.png",
  "xd3n2c08.png",
  "xd9n2c08.png",
  "xdtn0g01.png",
  "xhdn0g08.png",
  "xlfn0g04.png",
  "xs1n0g01.png",
  "xs2n0g01.png",
  "xs4n0g01.png",
  "xs7n0g01.png",
  "z00n2c08.png",
  "z03n2c08.png",
  "z06n2c08.png",
  "z09n2c08.png"
};

int submain() {
  const char * path = "/d/new/pngtest/";
  for(int i = 0; i < sizeof(tests)/sizeof(char*); i++) {
    std::cout << i << '\t' << tests[i] << '\t';
    std::string name = std::string(path) + std::string(tests[i]);
    std::ifstream input(name.c_str(), std::ifstream::binary);
    try {
      Image img = readPng(input);
      std::cout << img;
    } catch(OhNo ohno) {
      std::cout << ohno << std::endl;
    }
  }
  return 0;
}
*/
