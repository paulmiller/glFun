#ifndef GL_UTIL_H
#define GL_UTIL_H

#include "mesh.h"

#include <GL/glew.h> // must precede gl, glfw
#include <GLFW/glfw3.h>

// returns false if there have been any GL errors
bool checkGL();

// build GLSL
GLuint loadShader(const char *sourceName, GLenum shaderType);
GLuint linkProgram(GLuint vertShaderId, GLuint fragShaderId);

// make vertex buffer objects from a mesh
GLuint vertVBO(const Mesh &m);
GLuint uvVBO(const Mesh &m);
GLuint normVBO(const Mesh &m);

// turn a png file into a GL texture
GLuint pngTex(const char *pngName);

#endif
