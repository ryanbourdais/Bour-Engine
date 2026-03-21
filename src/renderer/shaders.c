#include "shaders.h"
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../utils/helpers/file_reader.h"

void check_shader_compile_status(GLuint shader, const char* shader_name)
{
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        char *info_log = malloc((size_t)log_length);
        if(info_log != NULL)
        {
            glGetShaderInfoLog(shader, log_length, NULL, info_log);
            fprintf(stderr, "Error compiling shader %s: %s\n", shader_name, info_log);
            free(info_log);
        }
        else
        {
            fprintf(stderr, "Error compiling shader %s: Unable to allocate memory for info log\n", shader_name);
        }

        glDeleteShader(shader);
        exit(1);
    }
}

void load_shaders(GLuint *vs, GLuint *fs)
{
    
    struct ShaderBuffer vertex_shader_buffer = read_shader_file("src/renderer/shaders/test.vert");
    struct ShaderBuffer fragment_shader_buffer = read_shader_file("src/renderer/shaders/test.frag");
    if(vertex_shader_buffer.data == NULL || fragment_shader_buffer.data == NULL) {
        free_shader_buffer(&vertex_shader_buffer);
        free_shader_buffer(&fragment_shader_buffer);
        fprintf(stderr, "Failed to read shader files\n");
        exit(1);
    }
    GLint vertex_length = (GLint)vertex_shader_buffer.length;
    GLint fragment_length = (GLint)fragment_shader_buffer.length;
    const GLchar * const * vertex_shader_source = (const GLchar * const *)&vertex_shader_buffer.data;
    const GLchar * const * fragment_shader_source = (const GLchar * const *)&fragment_shader_buffer.data;
    
    *vs = glCreateShader(GL_VERTEX_SHADER);
    *fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(*vs, 1, vertex_shader_source, &vertex_length);
    glCompileShader(*vs);
    check_shader_compile_status(*vs, "Vertex Shader");

    
    glShaderSource(*fs, 1, fragment_shader_source, &fragment_length);
    glCompileShader(*fs);
    check_shader_compile_status(*fs, "Fragment Shader");
    free_shader_buffer(&vertex_shader_buffer);
    free_shader_buffer(&fragment_shader_buffer);
}

void create_shader_program(GLuint *vs, GLuint *fs, GLuint *shader_program)
{
    *shader_program = glCreateProgram();
    glAttachShader(*shader_program, *fs);
    glAttachShader(*shader_program, *vs);
    glLinkProgram(*shader_program);
    glDeleteShader(*vs);
    glDeleteShader(*fs);
}