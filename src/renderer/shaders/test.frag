#version 410 core

out vec4 frag_colour;

in vec3 colour;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
  vec4 tex1 = texture(texture1, TexCoord);
  vec4 tex2 = texture(texture2, TexCoord);
  frag_colour = mix(tex1, tex2, tex2.a);
}