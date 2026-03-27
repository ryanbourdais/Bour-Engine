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