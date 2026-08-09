#pragma once

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct InstancedModel 
{
    mat4 *transforms;
    size_t count; 
    size_t capacity;

    GLuint instance_vbo;
} InstancedModel;

int instanced_model_init(InstancedModel *instances);
void instanced_model_free(InstancedModel *instances);

bool instanced_model_add_transform(InstancedModel *instances, mat4 transform);

void instanced_model_upload_transforms(InstancedModel *instances);