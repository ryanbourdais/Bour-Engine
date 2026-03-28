#include "mesh.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

//triangle vertices
float default_points[] = {
    0.0f, 0.5f, 0.0f, //top x,y,z
    0.5f, -0.5f, 0.0f, //bottom right x,y,z
    -0.5f, -0.5f, 0.0f //bottom left x,y,z
};

float default_colours[] = {
  1.0f, 0.0f,  0.0f,
  0.0f, 1.0f,  0.0f,
  0.0f, 0.0f,  1.0f
};

GLuint create_vbo()
{
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), default_points, GL_STATIC_DRAW);
    return vbo;
}

GLuint create_vao(GLuint *vbo)
{
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    return vao;
}

void create_mesh(Mesh *mesh)
{
    mesh->position_vbo = create_vbo();
    mesh->vao = create_vao(&mesh->position_vbo);
}

//TODO: implement
int create_mesh_from_vertices(Mesh *mesh, const Vertex *vertices, size_t vertex_count, const unsigned int *indices, GLsizei index_count)
{
    if(mesh == NULL)
    {
        fprintf(stderr, "Error: mesh is null");
        return 1;
    }
    if(vertices == NULL)
    {
        fprintf(stderr, "Error: vertices are null");
        return 1;
    }
    if(vertex_count == 0)
    {   
        fprintf(stderr, "Error: vertex count is less than 1");
        return 1;
    }
    
    // TODO: make more efficient
    Vec3Array positions_arr;
    vec3_array_initialize(&positions_arr);
    Vec3Array colors_arr;
    vec3_array_initialize(&colors_arr);
    Vec2Array uv_arr;
    vec2_array_initialize(&uv_arr);

    for(size_t i = 0; i < vertex_count; i++)
    {
        vec3_array_append(&positions_arr, vertices[i].position);
        vec3_array_append(&colors_arr, vertices[i].color);
        vec2_array_append(&uv_arr, vertices[i].uv);
    }
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    GLuint positions_vbo = 0;
    glGenBuffers(1, &positions_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
    glBufferData(GL_ARRAY_BUFFER, positions_arr.count * sizeof(vec3s), positions_arr.items, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    GLuint colors_vbo = 0;
    glGenBuffers(1, &colors_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glBufferData(GL_ARRAY_BUFFER, colors_arr.count * sizeof(vec3s), colors_arr.items, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    GLuint uv_vbo = 0;
    glGenBuffers(1, &uv_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
    glBufferData(GL_ARRAY_BUFFER, uv_arr.count * sizeof(vec2s), uv_arr.items, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    GLuint ebo = 0;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(indices[0]), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    mesh->color_vbo = colors_vbo;
    mesh->position_vbo = positions_vbo;
    mesh->vao = vao;
    mesh->vertex_count = positions_arr.count;
    mesh->ebo = ebo;
    mesh->index_count = index_count;
    mesh->uv_vbo = uv_vbo;
    free_vec3_array(&positions_arr);
    free_vec3_array(&colors_arr);
    free_vec2_array(&uv_arr);
    return 0;
}

void mesh_pool_initialize(struct MeshPool *arr)
{
    arr->items = malloc(4 * sizeof(Mesh)); // Initial capacity of 4
    if(arr->items == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    arr->count = 0;
    arr->capacity = 4;
}

void mesh_pool_append(struct MeshPool *arr, Mesh value)
{
    if(arr->count == arr->capacity) {
            arr->capacity *= 2;
            arr->items = realloc(arr->items, arr->capacity * sizeof(Mesh));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        if(arr->capacity == 0) {
            arr->capacity = 4; // Initial capacity
            arr->items = malloc(arr->capacity * sizeof(Mesh));
            if(arr->items == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        arr->items[arr->count] = value;
        arr->count++;
}

void free_mesh_pool(struct MeshPool *arr)
{
    free(arr->items);
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
}