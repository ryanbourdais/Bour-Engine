#pragma once

#include <stddef.h>
#include <cglm/struct.h>

#include "../geometry/primitive_types.h"
#include "../geometry/programmable_mesh.h"

#define MAX_RENDERABLES 512

typedef enum RenderableGeometryType {
    RENDERABLE_GEOMETRY_MODEL = 0,
    RENDERABLE_GEOMETRY_PRIMITIVE,
    RENDERABLE_GEOMETRY_PROGRAMMABLE
} RenderableGeometryType;

typedef struct RenderableDrawData {
    RenderableGeometryType geometry_type;
    const char *model_path;
    BuiltinPrimitiveType primitive_type;
    unsigned int programmable_mesh_id;
    ProgrammableMesh *programmable_mesh;
    mat4s model_matrix;
} RenderableDrawData;
