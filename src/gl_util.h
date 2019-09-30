#ifndef GL_UTIL_H
#define GL_UTIL_H

#include "math/matrix.h"
#include "mesh.h"

#include <GL/glew.h> // must precede gl, glfw
#include <GLFW/glfw3.h>

// returns false if there have been any GL errors
bool checkGL();

// build GLSL
GLuint loadShader(const char *source_name, GLenum shader_type);
GLuint linkProgram(GLuint vert_shader_id, GLuint frag_shader_id);

void UniformMatrix(GLint location, const Matrix4x4f &m);

// make vertex buffer objects from a mesh
GLuint vertVBO(const TriMesh &m);
GLuint uvVBO(const TriMesh &m);
GLuint normVBO(const TriMesh &m);

// turn a png file into a GL texture
GLuint pngTex(const char *png_name);

#endif
