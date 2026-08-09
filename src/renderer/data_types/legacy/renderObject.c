#include "renderObject.h"
#include <stdio.h>
#include <stdlib.h>

void draw_render_object(struct RenderObject *render_object, GLint model_location)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_object->mesh.texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, render_object->mesh.texture2);
    glBindVertexArray(render_object->mesh.vao);
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (float *)render_object->model);
    glDrawElements(GL_TRIANGLES, render_object->mesh.index_count, GL_UNSIGNED_INT, 0);
}

void identity_model(struct RenderObject *render_object)
{
    glm_mat4_identity(render_object->model);
}

void translate_model_matrix(struct RenderObject *render_object, vec3s translate_vector)
{
    glm_translate(render_object->model, translate_vector.raw);
}

void rotate_model(struct RenderObject *render_object, float angle, vec3s axis)
{
    glm_rotate(render_object->model, angle, axis.raw);
}

void scale_model(struct RenderObject *render_object, vec3s scale_vector)
{
    glm_scale(render_object->model, scale_vector.raw);
}

void renderobject_array_initialize(struct RenderObjectArray *arr)
{
    arr->items = malloc(4 * sizeof(RenderObject)); // Initial capacity of 4
    if (arr->items == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    arr->count = 0;
    arr->capacity = 4;
}
void renderobject_array_append(struct RenderObjectArray *arr, RenderObject value)
{
    if (arr->count == arr->capacity)
    {
        arr->capacity *= 2;
        arr->items = realloc(arr->items, arr->capacity * sizeof(RenderObject));
        if (arr->items == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    if (arr->capacity == 0)
    {
        arr->capacity = 4; // Initial capacity
        arr->items = malloc(arr->capacity * sizeof(RenderObject));
        if (arr->items == NULL)
        {
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