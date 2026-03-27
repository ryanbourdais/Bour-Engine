#pragma once
#include <glad/glad.h>
#include <stddef.h>
#include "vertex.h"

typedef struct Mesh {
    GLuint vao;
    GLuint position_vbo;
    GLuint color_vbo;
    GLuint uv_vbo;
    GLsizei vertex_count;
    GLuint ebo;
    GLsizei index_count;
    GLuint texture;
    GLuint texture2;
    GLuint texture_count;
} Mesh;

typedef struct MeshPool {
    Mesh* items;
    size_t count;
    size_t capacity;
} MeshPool;

GLuint create_vao(GLuint *vbo);
GLuint create_vbo();
void create_mesh(Mesh *mesh);
int create_mesh_from_vertices(Mesh *mesh, const Vertex *vertices, size_t vertex_count, const unsigned int *indices, GLsizei index_count);
void mesh_pool_initialize(struct MeshPool *arr);
void mesh_pool_append(struct MeshPool *arr, Mesh value);
void free_mesh_pool(struct MeshPool *arr);
