
#include "primitive_mesh.h"
#include <stddef.h>
#include <stdio.h>

static const Vertex cube_vertices[] = {
    {{{-0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{ 0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{ 0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{-0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}},

    {{{ 0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}},
    {{{-0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}},
    {{{-0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}},
    {{{ 0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}},

    {{{-0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{-1.0f, 0.0f, 0.0f}}},
    {{{-0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{-1.0f, 0.0f, 0.0f}}},
    {{{-0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{-1.0f, 0.0f, 0.0f}}},
    {{{-0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{-1.0f, 0.0f, 0.0f}}},

    {{{ 0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}}},
    {{{ 0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}}},
    {{{ 0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{1.0f, 0.0f, 0.0f}}},
    {{{ 0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{1.0f, 0.0f, 0.0f}}},

    {{{-0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{ 0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{ 0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{-0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}}},

    {{{-0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, -1.0f, 0.0f}}},
    {{{ 0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, -1.0f, 0.0f}}},
    {{{ 0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, -1.0f, 0.0f}}},
    {{{-0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, -1.0f, 0.0f}}},
};

static const unsigned int cube_indices[] = {
    0,  1,  2,  2,  3,  0,
    4,  5,  6,  6,  7,  4,
    8,  9, 10, 10, 11,  8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
};

static const Vertex plane_vertices[] = {
    {{{-0.5f, 0.0f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{ 0.5f, 0.0f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{ 0.5f, 0.0f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{-0.5f, 0.0f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}}},
};

static const unsigned int plane_indices[] = {
    0, 1, 2,
    2, 3, 0,
};

static const Vertex quad_vertices[] = {
    {{{-0.5f, -0.5f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{ 0.5f, -0.5f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{ 0.5f,  0.5f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{-0.5f,  0.5f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}},
};

static const unsigned int quad_indices[] = {
    0, 1, 2,
    2, 3, 0,
};

static void primitive_mesh_data_assign(
    PrimitiveMeshData *out_data, 
    const Vertex *vertices, 
    size_t vertex_count,
    const unsigned int *indices,
    size_t index_count
)
{
    out_data->vertices = vertices;
    out_data->vertex_count = vertex_count;
    out_data->indices = indices;
    out_data->index_count = index_count;
}

bool primitive_mesh_get_data(BuiltinPrimitiveType type, PrimitiveMeshData *out_data)
{
    if (out_data == NULL)
    {
        return false;
    }

    switch (type)
    {
        case BUILTIN_PRIMITIVE_CUBE:
            primitive_mesh_data_assign(
                out_data, 
                cube_vertices, 
                sizeof(cube_vertices) / sizeof(cube_vertices[0]),
                cube_indices,
                sizeof(cube_indices) / sizeof(cube_indices[0])
            );
            return true;
        case BUILTIN_PRIMITIVE_PLANE:
            primitive_mesh_data_assign(
                out_data,
                plane_vertices, 
                sizeof(plane_vertices) / sizeof(plane_vertices[0]),
                plane_indices,
                sizeof(plane_indices) / sizeof(plane_indices[0])
            );
            return true;
        case BUILTIN_PRIMITIVE_QUAD:
            primitive_mesh_data_assign(
                out_data,
                quad_vertices,
                sizeof(quad_vertices) / sizeof(quad_vertices[0]),
                quad_indices,
                sizeof(quad_indices) / sizeof(quad_indices[0])
            );
            return true;
        case BUILTIN_PRIMITIVE_CYLINDER:
            fprintf(stderr, "Not yet implemented\n");
            return false;
        case BUILTIN_PRIMITIVE_UV_SPHERE:
            fprintf(stderr, "Not yet implemented\n");
            return false;
        default:
            primitive_mesh_data_assign(out_data, NULL, 0, NULL, 0);
            return false;
    }
}


