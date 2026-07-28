#pragma once

#include <glad/glad.h>
#include <cglm/cglm.h>

typedef struct Skybox
{
    GLuint vao;
    GLuint vbo;
    GLuint cubemap_texture;
    GLuint shader_program;

    GLint view_location;
    GLint projection_location;
    GLint skybox_location;
} Skybox;

int skybox_init(Skybox *skybox, const char *const faces[6]);
void skybox_draw(Skybox *skybox, mat4 projection, mat4 view);
void skybox_free(Skybox *skybox);
