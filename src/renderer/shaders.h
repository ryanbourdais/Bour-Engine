#include <glad/glad.h>

void check_shader_compile_status(GLuint shader, const char* shader_name);
void load_shaders(GLuint *vs, GLuint *fs);
void create_shader_program(GLuint *vs, GLuint *fs, GLuint *shader_program);