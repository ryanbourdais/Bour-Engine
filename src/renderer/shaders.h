#pragma once
#include <glad/glad.h>

int load_shaders(GLuint *vs, GLuint *fs);
int create_shader_program(GLuint *vs, GLuint *fs, GLuint *shader_program);
void reload_shader_program_from_files(GLuint* program, const char* vertex_shader_filename, const char* fragment_shader_filename );