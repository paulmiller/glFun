#version 330 core

uniform mat4 MVP;

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

out vec3 fragmentColor;
out vec2 UV;
out vec3 normal;

void main(){
  gl_Position = MVP * vec4(vertexPosition_modelspace,1);
  fragmentColor = vertexPosition_modelspace / 2.0 + 0.5;
  UV = vertexUV;
  normal = vertexNormal;
}
