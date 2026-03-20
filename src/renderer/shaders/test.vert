// vertex shader defines where each 3d vertex points should be placed in the display
const char* vertex_shader =
"#version 410 core\n"
"in vec3 vp;"
"void main() {"
"  gl_Position = vec4( vp, 1.0 );"
"}";
