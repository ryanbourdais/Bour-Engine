#include "file_reader.h"
#include <stdio.h>
#include <stdlib.h>

void free_shader_buffer(struct ShaderBuffer *buffer) {
    free(buffer->data);
    buffer->data = NULL;
    buffer->length = 0;
}

struct ShaderBuffer read_shader_file(const char *file_path) {
    struct ShaderBuffer buffer = {NULL, 0};

    FILE *file = fopen(file_path, "rb");
    if(file == NULL) {
        perror("Error opening shader file ");
        return buffer;
    }

    if(fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        perror("Error seeking to end of shader file");
        return buffer;
    }

    long file_size = ftell(file);
    if(file_size < 0) {
        fclose(file);
        perror("Error getting shader file size");
        return buffer;
    }

    rewind(file);

    buffer.data = malloc((size_t)file_size + 1);
    if(buffer.data == NULL) {
        fclose(file);
        fprintf(stderr, "Memory allocation failed for shader buffer\n");
        return buffer;
    }

    size_t bytes_read = fread(buffer.data, 1, (size_t)file_size, file);
    if(bytes_read != (size_t)file_size) {
        free(buffer.data);
        buffer.data = NULL;
        fclose(file);
        fprintf(stderr, "Error reading shader file: expected %zu bytes, got %zu bytes\n", (size_t)file_size, bytes_read);
        return buffer;
    }

    buffer.data[bytes_read] = '\0';
    buffer.length = bytes_read;

    fclose(file);

    return buffer;
}
