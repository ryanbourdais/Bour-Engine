#pragma once
#include <cglm/struct.h>
#include "mesh.h"

typedef struct RenderObject {
    Mesh mesh;
    vec3s position;
    vec3s rotation_axis;
    float rotation_angle;
    vec3s scale;
    mat4 model;
} RenderObject;

typedef struct RenderObjectArray {
    RenderObject *items;
    size_t count;
    size_t capacity;
} RenderObjectArray;

void renderobject_array_initialize(struct RenderObjectArray *arr);
void renderobject_array_append(struct RenderObjectArray *arr, RenderObject value);
void free_renderobject_array(struct RenderObjectArray *arr);