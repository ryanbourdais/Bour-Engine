#include "mesh.h"
#include <stdbool.h>

int load_texture(GLuint *texture, const char *file_location);
int load_texture_ex(GLuint *texture, const char *file_location, bool srgb);
int create_texture(Mesh *mesh, char* file_location);
int create_texture_ex(Mesh *mesh, char *file_location, bool srgb);
int create_solid_color_texture(GLuint *texture, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
