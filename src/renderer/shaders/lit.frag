#version 410 core
out vec4 frag_colour;

uniform vec3 lightColor;

void main() {
    frag_colour = vec4(lightColor, 1.0);
}