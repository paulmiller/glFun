// fragment shader which takes UV coordinates and normals from the vertex shader
// and lights the surface

#version 330 core

in vec3 normal;
in vec3 vert_color;

out vec3 color;

void main(){
  vec3 lightUnitVec = normalize(vec3(2, 3, 1));
  vec3 normalUnitVec = normalize(normal);
  float exposure = clamp(dot(normalUnitVec, lightUnitVec), 0, 1);
  color = vert_color * exposure;
}
