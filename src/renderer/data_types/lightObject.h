#pragma once
#include <glad/glad.h>
#include <cglm/struct.h>
#include <stddef.h>
#include "renderObject.h"

typedef struct LightObject
{
    vec3s position;
    vec3s color;
    bool has_visual;
    RenderObject visual;
} LightObject;

void point_light_object_init(LightObject *light, vec3s position, vec3s color);
void light_object_init_with_visual(LightObject *light, vec3s position, vec3s color, RenderObject visual);
