#pragma once
#include <stddef.h>
#include <cglm/struct.h>

typedef struct Vec3Array {
    vec3s *items;
    size_t count;
    size_t capacity;
} Vec3Array;


typedef struct Vec2Array {
    vec2s *items;
    size_t count;
    size_t capacity;
} Vec2Array;

void vec3_array_initialize(struct Vec3Array *arr);
void vec3_array_append(struct Vec3Array *arr, vec3s value);
void free_vec3_array(struct Vec3Array *arr);

void vec2_array_initialize(struct Vec2Array *arr);
void vec2_array_append(struct Vec2Array *arr, vec2s value);
void free_vec2_array(struct Vec2Array *arr);