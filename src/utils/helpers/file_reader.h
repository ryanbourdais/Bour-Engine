#include <stddef.h>

struct ShaderBuffer {
    char *data;
    size_t length;
};


struct ShaderBuffer read_shader_file(const char *file_path);
void free_shader_buffer(struct ShaderBuffer *buffer);