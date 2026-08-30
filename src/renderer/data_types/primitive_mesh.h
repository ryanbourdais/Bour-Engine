#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "../../ecs/components.h"
#include "vertex.h"

typedef struct PrimitiveMeshData {
    const Vertex *vertices;
    size_t vertex_count;
    const unsigned int *indices;
    size_t index_count;
} PrimitiveMeshData;

bool primitive_mesh_get_data(BuiltinPrimitiveType type, PrimitiveMeshData *out_data);
