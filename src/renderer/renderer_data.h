#pragma once

#include <stddef.h>
#include <cglm/struct.h>

#define MAX_RENDERABLES 512

typedef struct RenderableDrawData {
    const char *model_path;
    mat4s model_matrix;
} RenderableDrawData;