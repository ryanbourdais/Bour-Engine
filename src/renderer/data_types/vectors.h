#pragma once
#include <stddef.h>

typedef struct Vec2{
    float x;
    float y;
} Vec2;

typedef struct Vec3{
    float x;
    float y;
    float z;
} Vec3;

typedef struct Vec4{
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct Vec3Array {
    Vec3 *items;
    size_t count;
    size_t capacity;
} Vec3Array;


typedef struct Vec2Array {
    Vec2 *items;
    size_t count;
    size_t capacity;
} Vec2Array;

void vec3_array_initialize(struct Vec3Array *arr);
void vec3_array_append(struct Vec3Array *arr, Vec3 value);
void free_vec3_array(struct Vec3Array *arr);

void vec2_array_initialize(struct Vec2Array *arr);
void vec2_array_append(struct Vec2Array *arr, Vec2 value);
void free_vec2_array(struct Vec2Array *arr);