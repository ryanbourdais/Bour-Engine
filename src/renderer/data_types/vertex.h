#pragma once
#include <glad/glad.h>
#include <stddef.h>
#include "vectors.h"


typedef struct Vertex {
    Vec3 position;
    Vec3 color;
    Vec2 uv;
    // Vec3 normal;
} Vertex;

typedef struct VertexArray {
    Vertex *items;
    size_t count;
    size_t capacity;
} VertexArray;

void vertex_array_initialize(struct VertexArray *arr);
void vertex_array_append(struct VertexArray *arr, Vertex value);
void free_vertex_array(struct VertexArray *arr);