#version 410 core

layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 vc; 
layout (location = 2) in vec2 aTexCoord;

// uniform float time;
// uniform float mov_x;
// uniform float mov_y;
uniform mat4 model;

out vec3 colour;
out vec2 TexCoord;

void main() {
  vec3 pos = vp;
  // pos.y += sin(time);
  // pos.x += mov_x;
  // pos.y += mov_y;
  gl_Position = model * vec4( pos, 1.0 );
  colour = vc;
  TexCoord = aTexCoord;
}