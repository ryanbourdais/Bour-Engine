#include "vectors.h"
#include <stdio.h>
#include <stdlib.h>

void vec3_array_initialize(struct Vec3Array *arr)
{
    arr->items = malloc(4 * sizeof(Vec3)); // Initial capacity of 4
    if(arr->items == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    arr->count = 0;
    arr->capacity = 4;
}
void vec3_array_append(struct Vec3Array *arr, Vec3 value)
{
    if(arr->count == arr->capacity) {
            arr->capacity *= 2;
            arr->items = realloc(arr->items, arr->capacity * sizeof(Vec3));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        if(arr->capacity == 0) {
            arr->capacity = 4; // Initial capacity
            arr->items = malloc(arr->capacity * sizeof(Vec3));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        arr->items[arr->count] = value;
        arr->count++;
}

void free_vec3_array(struct Vec3Array *arr)
{
    free(arr->items);
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void vec2_array_initialize(struct Vec2Array *arr)
{
    arr->items = malloc(4 * sizeof(Vec2)); // Initial capacity of 4
    if(arr->items == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    arr->count = 0;
    arr->capacity = 4;
}
void vec2_array_append(struct Vec2Array *arr, Vec2 value)
{
    if(arr->count == arr->capacity) {
            arr->capacity *= 2;
            arr->items = realloc(arr->items, arr->capacity * sizeof(Vec2));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        if(arr->capacity == 0) {
            arr->capacity = 4; // Initial capacity
            arr->items = malloc(arr->capacity * sizeof(Vec2));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        arr->items[arr->count] = value;
        arr->count++;
}

void free_vec2_array(struct Vec2Array *arr)
{
    free(arr->items);
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}


void vec3_scalar_add(Vec3 *vertex, float addenda)
{
    vertex->x = vertex->x + addenda;
    vertex->y = vertex->y + addenda;
    vertex->z = vertex->z + addenda;
}


void vec3_scalar_subtraction(Vec3 *vertex, float minuend)
{
    vertex->x = vertex->x - minuend;
    vertex->y = vertex->y - minuend;
    vertex->z = vertex->z - minuend;
}

Vec3 vec3_add(Vec3 vertex, Vec3 addenda_vertex)
{
    Vec3 new_vec3;
    new_vec3.x = vertex.x + addenda_vertex.x;
    new_vec3.y = vertex.y + addenda_vertex.y;
    new_vec3.z = vertex.z + addenda_vertex.z;
    return new_vec3;
}

Vec3 vec3_subtraction(Vec3 vertex, Vec3 minuend_vertex)
{
    Vec3 new_vec3;
    new_vec3.x = vertex.x - minuend_vertex.x;
    new_vec3.y = vertex.y - minuend_vertex.y;
    new_vec3.z = vertex.z - minuend_vertex.z;
    return new_vec3;
}

float vec3_dot_product(Vec3 vertex, Vec3 mulitplier_vertex)
{
    Vec3 new_vec3;
    new_vec3.x = vertex.x * mulitplier_vertex.x;
    new_vec3.y = vertex.y * mulitplier_vertex.y;
    new_vec3.z = vertex.z * mulitplier_vertex.z;
    float dot_product = new_vec3.x + new_vec3.y + new_vec3.z;
    return dot_product;
}