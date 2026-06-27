#include "texture.h"
#include "../../external/stb/stb_image.h"

static int init_textures()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return 0;
}

int create_texture(Mesh *mesh, char* file_location)
{
    GLuint texture = 0;

    if (load_texture(&texture, file_location) != 0)
    {
        return 1;
    }

    if (!mesh->texture)
    {
        mesh->texture = texture;
    }
    else
    {
        mesh->texture2 = texture;
    }

    return 0;
}

int load_texture(GLuint *out_texture, const char *file_location)
{
    int width, height, nrChannels;

    stbi_set_flip_vertically_on_load(0);

    unsigned char *data = stbi_load(file_location, &width, &height, &nrChannels, 0);

    if (data == NULL)
    {
        fprintf(
            stderr,
            "Texture faield to load: %s reason: %s\n",
            file_location,
            stbi_failure_reason()
        );
        return 1;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (init_textures() != 0)
    {
        stbi_image_free(data);
        return 1;
    }

    if (nrChannels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (nrChannels == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        fprintf(stderr, "Unsupported texture channel count %d for %s\n", nrChannels, file_location);
        stbi_image_free(data);
        glDeleteTextures(1, &texture);
        return 1;
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    *out_texture = texture;

    return 0;
}

int create_solid_color_texture(GLuint *out_texture, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    unsigned char pixel[4] = {r, g, b, a};

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (init_textures() != 0)
    {
        return 1;
    }

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        1,
        1,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixel
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    *out_texture = texture;

    return 0;
}