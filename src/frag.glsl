#version 330 core

uniform sampler2D myTextureSampler;

in vec3 fragmentColor;
in vec2 UV;
in vec3 normal;

out vec3 color;

void main(){
  //color = fragmentColor;
  //color = texture(myTextureSampler, UV).rgb;
  //color = vec3(UV.x, UV.y, 0.0);
  //color = normal;

  vec3 surfaceColor = texture(myTextureSampler, UV).rgb;
  vec3 light = normalize(vec3(1, 1, 1));
  vec3 normal1 = normalize(normal);
  float exposure = clamp(dot(normal1, light), 0, 1);
  color = surfaceColor * exposure;
}
