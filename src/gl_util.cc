#include "gl_util.h"

#include "image.h"
#include "image_png.h"
#include "ohno.h"
#include "util.h"

#include <cassert>
#include <fstream>
#include <vector>

bool checkGL() {
  bool no_errors = true;
  while(no_errors) {
    switch(glGetError()) {
      case GL_NO_ERROR:
        return no_errors;
      case GL_INVALID_ENUM:
        std::cout << "GL_INVALID_ENUM" << std::endl;
        no_errors = false;
        break;
      case GL_INVALID_VALUE:
        std::cout << "GL_INVALID_VALUE" << std::endl;
        no_errors = false;
        break;
      case GL_INVALID_OPERATION:
        std::cout << "GL_INVALID_OPERATION" << std::endl;
        no_errors = false;
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
        no_errors = false;
        break;
      case GL_OUT_OF_MEMORY:
        std::cout << "GL_OUT_OF_MEMORY" << std::endl;
        no_errors = false;
        break;
      case GL_STACK_UNDERFLOW:
        std::cout << "GL_STACK_UNDERFLOW" << std::endl;
        no_errors = false;
        break;
      case GL_STACK_OVERFLOW:
        std::cout << "GL_STACK_OVERFLOW" << std::endl;
        no_errors = false;
        break;
      default:
        std::cout << "unknown gl error" << std::endl;
        no_errors = false;
    }
  }
  return no_errors;
}

namespace {
  // load a text file and append a null-terminator
  std::vector<char> loadShaderFile(const char *file_name) {
    std::ifstream in(file_name);
    if(in.bad())
      throw OHNO("couldn't open file");

    in.seekg(0, in.end);
    size_t size = in.tellg();
    in.seekg(0, in.beg);

    std::vector<char> buffer(size + 1); // init to 0. +1 for null.

    in.read(buffer.data(), size);
    if(in.bad())
      throw OHNO("couldn't read file");

    return buffer;
  }
}

GLuint loadShader(const char *file_name, GLenum shader_type) {
  assert(shader_type == GL_VERTEX_SHADER || shader_type == GL_FRAGMENT_SHADER);

  GLuint id = glCreateShader(shader_type);

  COMPILE_ASSERT(sizeof(char) == sizeof(GLchar));
  std::vector<char> source = loadShaderFile(file_name);
  const char * const data_ptr = source.data();
  glShaderSource(id, 1, &data_ptr, nullptr);

  glCompileShader(id);

  GLint result = GL_FALSE;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE) {
    GLint log_length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);
    std::vector<char> log(log_length);
    glGetShaderInfoLog(id, log_length, nullptr, log.data());
    std::cout << "failed compiling shader \"" << file_name << "\":\n"
      << log.data();
  }

  assert(checkGL());

  return id;
}

GLuint linkProgram(GLuint vert_shader_id, GLuint frag_shader_id) {
  GLuint program_id = glCreateProgram();
  glAttachShader(program_id, vert_shader_id);
  glAttachShader(program_id, frag_shader_id);
  glLinkProgram(program_id);

  GLint result = GL_FALSE;
  glGetProgramiv(program_id, GL_LINK_STATUS, &result);
  if(result == GL_FALSE) {
    GLint log_length;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
    std::vector<char> log(log_length);
    glGetProgramInfoLog(program_id, log_length, nullptr, log.data());
    std::cout << "failed linking program:\n" << log.data();
  }

  assert(checkGL());

  return program_id;
}

GLuint vertVBO(const TriMesh &m) {
  // 3 verts per tri, 3 floats per vert
  int float_num = m.tris.size() * 3 * 3;
  std::unique_ptr<GLfloat[]> vbo(new GLfloat[float_num]);

  int b = 0;
  for(auto it = m.tris.begin(); it != m.tris.end(); it++) {
    for(int i = 0; i < 3; i++) {
      const Vec3 &v = m.verts[it->vert_idxs[i]];
      vbo[b++] = v.x;
      vbo[b++] = v.y;
      vbo[b++] = v.z;
    }
  }

  GLuint vert_buffer_id;
  glGenBuffers(1, &vert_buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, vert_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, float_num * sizeof(GLfloat), vbo.get(),
    GL_STATIC_DRAW);

  return vert_buffer_id;
}

GLuint uvVBO(const TriMesh &m) {
  // 3 verts per tri, 2 floats per vert
  int float_num = m.tris.size() * 3 * 2;
  std::unique_ptr<GLfloat[]> vbo(new GLfloat[float_num]);

  int b = 0;
  for(auto it = m.tris.begin(); it != m.tris.end(); it++) {
    for(int i = 0; i < 3; i++) {
      const UVCoord &v = m.uvs[it->uv_idxs[i]];
      vbo[b++] = v.u;
      vbo[b++] = v.v;
    }
  }

  GLuint uv_buffer_id;
  glGenBuffers(1, &uv_buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, uv_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, float_num * sizeof(GLfloat), vbo.get(),
    GL_STATIC_DRAW);

  return uv_buffer_id;
}

GLuint normVBO(const TriMesh &m) {
  // 3 normals per tri, 3 floats per normal
  int float_num = m.tris.size() * 3 * 3;
  std::unique_ptr<GLfloat[]> vbo(new GLfloat[float_num]);

  int b = 0;
  for(auto it = m.tris.begin(); it != m.tris.end(); it++) {
    for(int i = 0; i < 3; i++) {
      const Vec3 &n = m.normals[it->normal_idxs[i]];
      vbo[b++] = n.x;
      vbo[b++] = n.y;
      vbo[b++] = n.z;
    }
  }

  GLuint norm_buffer_id;
  glGenBuffers(1, &norm_buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, norm_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, float_num * sizeof(GLfloat), vbo.get(),
    GL_STATIC_DRAW);

  return norm_buffer_id;
}

namespace {
  // https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.9 pg 226
  GLuint px2glInternalFormat(Pixel::Type t) {
    switch(t) {
      case Pixel::V8_T:     return GL_R8;
      case Pixel::V16_T:    return GL_R16;
      case Pixel::RGB8_T:   return GL_RGBA8;
      case Pixel::RGB16_T:  return GL_RGBA16;
      case Pixel::RGBA8_T:  return GL_RGBA8;
      case Pixel::RGBA16_T: return GL_RGBA16;

      case Pixel::NONE_T:
      case Pixel::VA8_T:
      case Pixel::VA16_T:
        throw OHNO("bad pixel type");

      default:
        assert(0);
        return 0;
    }
  }

  // https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.3 pg 183
  GLuint px2glFormat(Pixel::Type t) {
    switch(t) {
      case Pixel::V8_T:
      case Pixel::V16_T:
        return GL_RED;
      case Pixel::RGB8_T:
      case Pixel::RGB16_T:
        return GL_RGB;
      case Pixel::RGBA8_T:
      case Pixel::RGBA16_T:
        return GL_RGBA;

      case Pixel::NONE_T:
      case Pixel::VA8_T:
      case Pixel::VA16_T:
        throw OHNO("bad pixel type");

      default:
        assert(0);
        return 0;
    }
  }

  // https://www.opengl.org/registry/doc/glspec45.core.pdf table 8.2 pg 182
  GLuint px2glType(Pixel::Type t) {
    switch(t) {
      case Pixel::V8_T:
      case Pixel::VA8_T:
      case Pixel::RGB8_T:
      case Pixel::RGBA8_T:
        return GL_UNSIGNED_BYTE;
      case Pixel::V16_T:
      case Pixel::VA16_T:
      case Pixel::RGB16_T:
      case Pixel::RGBA16_T:
        return GL_UNSIGNED_SHORT;

      case Pixel::NONE_T:
        throw OHNO("bad pixel type");

      default:
        assert(0);
        return 0;
    }
  }
}

GLuint pngTex(const char *png_name) {
  std::ifstream input(png_name, std::ifstream::binary);
  Image tex_img = readPng(input);
  Pixel::Type type = tex_img.type();

  GLuint tex_id;
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(
    GL_TEXTURE_2D,             // target
    0,                         // level
    px2glInternalFormat(type), // internalFormat
    tex_img.width(),            // width
    tex_img.height(),           // height
    0,                         // border
    px2glFormat(type),         // format
    px2glType(type),           // type
    tex_img.data()              // data
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  return tex_id;
}
