#include "gl_util.h"

#include "bytes.h"
#include "image.h"
#include "image_png.h"
#include "ohno.h"
#include "util.h"

#include <cassert>
#include <fstream>

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

  GLuint uvBufferId;
  glGenBuffers(1, &uvBufferId);
  glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
  glBufferData(GL_ARRAY_BUFFER, floatNum * sizeof(GLfloat), vbo,
    GL_STATIC_DRAW);

  delete[] vbo;
  return uvBufferId;
}

GLuint normVBO(const Mesh &m) {
  // 3 normals per tri, 3 floats per normal
  int floatNum = m.mTris.size() * 3 * 3;
  GLfloat *vbo = new GLfloat[floatNum];

  int b = 0;
  for(auto it = m.mTris.begin(); it != m.mTris.end(); it++) {
    for(int i = 0; i < 3; i++) {
      const Vec3 &n = m.mNormals[it->normalIdxs[i]];
      vbo[b++] = n.x;
      vbo[b++] = n.y;
      vbo[b++] = n.z;
    }
  }

  GLuint normBufferId;
  glGenBuffers(1, &normBufferId);
  glBindBuffer(GL_ARRAY_BUFFER, normBufferId);
  glBufferData(GL_ARRAY_BUFFER, floatNum * sizeof(GLfloat), vbo,
    GL_STATIC_DRAW);

  delete[] vbo;
  return normBufferId;
}

namespace {
  // https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.9 pg 226
  GLuint px2glInternalFormat(Pixel::Type t) {
    switch(t) {
      case Pixel::V8:     return GL_R8;
      case Pixel::V16:    return GL_R16;
      case Pixel::RGB8:   return GL_RGBA8;
      case Pixel::RGB16:  return GL_RGBA16;
      case Pixel::RGBA8:  return GL_RGBA8;
      case Pixel::RGBA16: return GL_RGBA16;

      case Pixel::NONE:
      case Pixel::VA8:
      case Pixel::VA16:
        throw OHNO("bad pixel type");

      default:
        assert(0);
        return 0;
    }
  }

  // https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.3 pg 183
  GLuint px2glFormat(Pixel::Type t) {
    switch(t) {
      case Pixel::V8:
      case Pixel::V16:
        return GL_RED;
      case Pixel::RGB8:
      case Pixel::RGB16:
        return GL_RGB;
      case Pixel::RGBA8:
      case Pixel::RGBA16:
        return GL_RGBA;

      case Pixel::NONE:
      case Pixel::VA8:
      case Pixel::VA16:
        throw OHNO("bad pixel type");

      default:
        assert(0);
        return 0;
    }
  }

  // https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.2 pg 182
  GLuint px2glType(Pixel::Type t) {
    switch(t) {
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

      case Pixel::NONE:
        throw OHNO("bad pixel type");

      default:
        assert(0);
        return 0;
    }
  }
}

GLuint pngTex(const char *pngName) {
  std::ifstream input(pngName, std::ifstream::binary);
  Image texImg = loadPng(input);
  Pixel::Type type = texImg.type();

  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(
    GL_TEXTURE_2D,             // target
    0,                         // level
    px2glInternalFormat(type), // internalFormat
    texImg.width(),            // width
    texImg.height(),           // height
    0,                         // border
    px2glFormat(type),         // format
    px2glType(type),           // type
    texImg.data()              // data
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  return texId;
}
