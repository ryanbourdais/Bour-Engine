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

void identity_model(struct RenderObject *render_object);
void translate_model_matrix(struct RenderObject *render_object, vec3s translate_vector);
void rotate_model(struct RenderObject *render_object, float angle, vec3s axis);
void scale_model(struct RenderObject *render_object, vec3s scale_vector);
void renderobject_array_initialize(struct RenderObjectArray *arr);
void renderobject_array_append(struct RenderObjectArray *arr, RenderObject value);
void free_renderobject_array(struct RenderObjectArray *arr);