#pragma once

#include <stdbool.h>

#include "../../geometry/primitive_types.h"
#include "mesh.h"

typedef struct PrimitiveMeshResources {
    Mesh meshes[BUILTIN_PRIMITIVE_COUNT];
    bool loaded[BUILTIN_PRIMITIVE_COUNT];
} PrimitiveMeshResources;

void primitive_mesh_resources_init(
    PrimitiveMeshResources *resources
);

bool primitive_mesh_resources_upload(
    PrimitiveMeshResources *resources
);

const Mesh *primitive_mesh_resources_get(
    const PrimitiveMeshResources *resources,
    BuiltinPrimitiveType type
);

void primitive_mesh_resources_free(
    PrimitiveMeshResources *resources
);
