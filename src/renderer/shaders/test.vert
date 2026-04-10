#version 410 core

layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 vc; 
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 colour;
out vec2 TexCoord;

void main() {
  vec3 pos = vp;
  gl_Position = projection * view * model * vec4(pos, 1.0);
  colour = vc;
  TexCoord = aTexCoord;
}