#version 410 core

layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 vc; 
// uniform float time;
uniform float mov_x;
uniform float mov_y;

out vec3 colour;

void main() {
  vec3 pos = vp;
  // pos.y += sin(time);
  pos.x += mov_x;
  pos.y += mov_y;
  gl_Position = vec4( pos, 1.0 );
  colour = vc;
}
