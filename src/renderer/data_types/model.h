#pragma once

#include <stddef.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "mesh.h"



typedef struct Model
{
    Mesh *meshes;
    size_t count;
    size_t capacity;
} Model;

void model_init(Model *model);
int model_load_gltf(Model *model, const char *path);
void model_free(Model *model);
void draw_model(Model *model, GLint model_location, mat4 model_matrix);