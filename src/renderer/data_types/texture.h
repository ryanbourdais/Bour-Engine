#include "mesh.h"


int load_texture(GLuint *texture, const char *file_location);
int create_texture(Mesh *mesh, char* file_location);
int create_solid_color_texture(GLuint *texture, unsigned char r, unsigned char g, unsigned char b, unsigned char a);