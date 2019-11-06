#version 330 core

uniform mat4 mvp;

layout(location = 0) in vec3 vertPosModelSpace;

void main(){
  gl_Position = mvp * vec4(vertPosModelSpace,1);
}
