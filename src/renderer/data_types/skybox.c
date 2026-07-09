#include "skybox.h"

#include <stdio.h>
#include <string.h>

#include "../../external/stb/stb_image.h"
#include "../shaders.h"

static float skybox_vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

static int load_cubemap(GLuint *out_texture, const char *faces[6])
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    stbi_set_flip_vertically_on_load(0);

    for (int i = 0; i < 6; i++)
    {
        int width;
        int height;
        int channels;

        unsigned char *data = stbi_load(faces[i], &width, &height, &channels, 0);

        if (data == NULL)
        {
            fprintf(stderr, "Failed to load cubemap face %s: %s\n", faces[i], stbi_failure_reason());
            glDeleteTextures(1, &texture);
            return 1;
        }

        GLenum source_format = GL_RGB;
        GLenum internal_format = GL_SRGB;

        if (channels == 4)
        {
            source_format = GL_RGBA;
            internal_format = GL_SRGB_ALPHA;
        }
        else if (channels == 3)
        {
            source_format = GL_RGB;
            internal_format = GL_SRGB;
        }
        else {
            fprintf(stderr, "Unsupported cubemap channel count %d for %s\n", channels, faces[i]);
            stbi_image_free(data);
            glDeleteTextures(1, &texture);
            return 1;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width, height, 0, source_format, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    *out_texture = texture;

    return 0;
}

int skybox_init(Skybox *skybox, const char *faces[6])
{
    memset(skybox, 0, sizeof(Skybox));

    GLuint vs;
    GLuint fs;

    if (load_shaders(&vs, &fs, "src/renderer/shaders/skybox.vert", "src/renderer/shaders/skybox.frag") != 0)
    {
        return 1;
    }

    if (create_shader_program(&vs, &fs, &skybox->shader_program) != 0)
    {
        return 1;
    }

    if (load_cubemap(&skybox->cubemap_texture, faces) != 0)
    {
        return 1;
    }

    glGenVertexArrays(1, &skybox->vao);
    glGenBuffers(1, &skybox->vbo);

    glBindVertexArray(skybox->vao);

    glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    skybox->view_location = glGetUniformLocation(skybox->shader_program, "view");
    skybox->projection_location = glGetUniformLocation(skybox->shader_program, "projection");
    skybox->skybox_location = glGetUniformLocation(skybox->shader_program, "skybox");

    glUseProgram(skybox->shader_program);
    glUniform1i(skybox->skybox_location, 0);

    glBindVertexArray(0);

    return 0;
}

void skybox_draw(Skybox *skybox, mat4 projection, mat4 view)
{
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glUseProgram(skybox->shader_program);

    mat4 skybox_view;
    glm_mat4_copy(view, skybox_view);

    skybox_view[3][0] = 0.0f;
    skybox_view[3][1] = 0.0f;
    skybox_view[3][2] = 0.0f;

    glUniformMatrix4fv(skybox->view_location, 1, GL_FALSE, (float *)skybox_view);
    glUniformMatrix4fv(skybox->projection_location, 1, GL_FALSE, (float *)projection);

    glBindVertexArray(skybox->vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->cubemap_texture);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void skybox_free(Skybox *skybox)
{
    glDeleteVertexArrays(1, &skybox->vao);
    glDeleteBuffers(1, &skybox->vbo);
    glDeleteTextures(1, &skybox->cubemap_texture);
    glDeleteProgram(skybox->shader_program);
}