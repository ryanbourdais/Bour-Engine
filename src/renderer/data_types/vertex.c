#include "vertex.h"
#include <stdio.h>
#include <stdlib.h>


void vertex_array_initialize(struct VertexArray *arr)
{
    arr->items = malloc(4 * sizeof(Vertex)); // Initial capacity of 4
    if(arr->items == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    arr->count = 0;
    arr->capacity = 4;
}
void vertex_array_append(struct VertexArray *arr, Vertex value)
{
    if(arr->count == arr->capacity) {
            arr->capacity *= 2;
            arr->items = realloc(arr->items, arr->capacity * sizeof(Vertex));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        if(arr->capacity == 0) {
            arr->capacity = 4; // Initial capacity
            arr->items = malloc(arr->capacity * sizeof(Vertex));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        arr->items[arr->count] = value;
        arr->count++;
}

void free_vertex_array(struct VertexArray *arr)
{
    free(arr->items);
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}