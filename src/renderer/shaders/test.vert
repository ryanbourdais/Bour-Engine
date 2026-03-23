#version 410 core

in vec3 vp;
// uniform float time;
uniform float mov_x;
uniform float mov_y;
out vec3 pos;

void main() {
  pos = vp;
  // pos.y += sin(time);
  pos.x += mov_x;
  pos.y += mov_y;
  gl_Position = vec4( pos, 1.0 );
}
