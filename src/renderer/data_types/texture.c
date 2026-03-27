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

int create_texture(Mesh *mesh, size_t vertex_count, char* file_location)
{
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(1);  
    unsigned char *data = stbi_load(file_location, &width, &height, &nrChannels, 0);

    unsigned int texture;
    glGenTextures(1, &texture);         
    glBindTexture(GL_TEXTURE_2D, texture);
    if(init_textures() != 0) {return 1;}
    if (data && nrChannels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else if(data && nrChannels == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        fprintf(stderr, "Texture failed to load");
        return 1;
    }
    stbi_image_free(data);
    if(!mesh->texture)
    {
        mesh->texture = texture;
    }
    else
    {
        mesh->texture2 = texture;
    }
    return 0;
}