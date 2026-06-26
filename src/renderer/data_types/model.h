#pragma once

#include <stddef.h>
#include <glad/glad.h>
#include <cglm/cglm.h>

#include "material.h"
#include "mesh.h"


typedef struct TextureCacheEntry {
    char path[512];
    GLuint texture;
} TextureCacheEntry;

typedef struct ModelMesh {
    Mesh mesh;
    Material material;
    mat4 transform;
} ModelMesh;

typedef struct Model
{
    ModelMesh *meshes;
    size_t count;
    size_t capacity;
    TextureCacheEntry *texture_cache;
    size_t texture_cache_count;
    size_t texture_cache_capacity;
} Model;

void model_init(Model *model);
int model_load_gltf(Model *model, const char *path);
void model_free(Model *model);
void draw_model(Model *model, GLint model_location, MaterialUniforms *material_uniforms, mat4 model_matrix);