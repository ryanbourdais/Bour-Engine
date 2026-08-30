#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "../../ecs/components.h"
#include <cglm/struct.h>


typedef struct Vertex {
    vec3s position;
    vec3s color;
    vec2s uv;
    vec3s normal;
} Vertex;
