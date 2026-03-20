// fragment shader defines the color of each pixel of the mesh
const char* fragment_shader =
"#version 410 core\n"
"out vec4 frag_colour;"
"void main() {"
"  frag_colour = vec4( 0.5, 0.0, 0.5, 1.0 );"
"}";