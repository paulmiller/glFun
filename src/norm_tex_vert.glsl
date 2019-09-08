// vertex shader which takes vertrex positions, UV coordinates, and normals from
// VBOs, applies model-view-projection transform to the position, and forwards
// UVs and normals to the fragment shader

#version 330 core

uniform mat4 mvp;

layout(location = 0) in vec3 vertPosModelSpace;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertUv;

out vec3 normal;
out vec2 uv;

void main(){
  gl_Position = mvp * vec4(vertPosModelSpace,1);
  normal = vertNormal;
  uv = vertUv;
}
