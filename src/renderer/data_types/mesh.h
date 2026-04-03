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
    // TODO(render-object/material): These texture handles are fine for now, but long term
    // appearance data should move off Mesh so one mesh can be reused by many render objects/materials.
    GLuint texture;
    GLuint texture2;
    GLuint texture_count;
} Mesh;

int create_mesh_from_vertices(Mesh *mesh, const Vertex *vertices, size_t vertex_count, const unsigned int *indices, GLsizei index_count);