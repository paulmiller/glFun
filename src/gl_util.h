#ifndef GL_UTIL_H
#define GL_UTIL_H

#include "math/matrix.h"
#include "mesh.h"
#include "ray.h"

#include <GL/glew.h> // must precede gl, glfw
#include <GLFW/glfw3.h>

#include <utility>
#include <vector>

// returns false if there have been any GL errors
bool CheckGl();

// build GLSL
GLuint LoadShader(const char *source_name, GLenum shader_type);
GLuint LinkProgram(GLuint vert_shader_id, GLuint frag_shader_id);

void UniformMatrix(GLint location, const Matrix4x4f &m);

// make vertex buffer objects from a mesh
GLuint MakeVertexVbo(const TriMesh &m);
GLuint MakeUvVbo(const TriMesh &m);
GLuint MakeNormVbo(const TriMesh &m);
GLuint MakeColorVbo(const TriMesh &m);
GLuint MakeLinesVbo(const std::vector<std::pair<Vector3f,Vector3f>> &lines);
GLuint MakeRaysVbo(const std::vector<Ray> &rays);

// turn a png file into a GL texture
GLuint MakeTextureFromPng(const char *png_name);

#endif
