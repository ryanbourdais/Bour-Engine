#include "mesh.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

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
    Vec3Array norm_arr;
    vec3_array_initialize(&norm_arr);

    for(size_t i = 0; i < vertex_count; i++)
    {
        vec3_array_append(&positions_arr, vertices[i].position);
        vec3_array_append(&colors_arr, vertices[i].color);
        vec2_array_append(&uv_arr, vertices[i].uv);
        vec3_array_append(&norm_arr, vertices[i].normal);
    }
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
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
    GLuint norm_vbo = 0;
    glGenBuffers(1, &norm_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, norm_vbo);
    glBufferData(GL_ARRAY_BUFFER, norm_arr.count * sizeof(vec3s), norm_arr.items, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, norm_vbo);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    GLuint ebo = 0;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(indices[0]), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    mesh->color_vbo = colors_vbo;
    mesh->position_vbo = positions_vbo;
    mesh->normal_vbo = norm_vbo;
    mesh->vao = vao;
    mesh->vertex_count = positions_arr.count;
    mesh->ebo = ebo;
    mesh->index_count = index_count;
    mesh->uv_vbo = uv_vbo;
    free_vec3_array(&positions_arr);
    free_vec3_array(&colors_arr);
    free_vec2_array(&uv_arr);
    free_vec3_array(&norm_arr);
    return 0;
}
