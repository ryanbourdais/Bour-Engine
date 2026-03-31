#include "renderObject.h"
#include <stdio.h>
#include <stdlib.h>


void renderobject_array_initialize(struct RenderObjectArray *arr)
{
  arr->items = malloc(4 * sizeof(RenderObject)); // Initial capacity of 4
    if(arr->items == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    arr->count = 0;
    arr->capacity = 4;
}
void renderobject_array_append(struct RenderObjectArray *arr, RenderObject value)
{
    if(arr->count == arr->capacity) {
            arr->capacity *= 2;
            arr->items = realloc(arr->items, arr->capacity * sizeof(RenderObject));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        if(arr->capacity == 0) {
            arr->capacity = 4; // Initial capacity
            arr->items = malloc(arr->capacity * sizeof(RenderObject));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        arr->items[arr->count] = value;
        arr->count++;
}
void free_renderobject_array(struct RenderObjectArray *arr)
{
    free(arr->items);
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}