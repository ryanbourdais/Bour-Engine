#include "mesh.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

int create_mesh_from_vertices(Mesh *mesh, const Vertex *vertices, size_t vertex_count, const unsigned int *indices, GLsizei index_count)
{
    if (mesh == NULL)
    {
        fprintf(stderr, "Error: mesh is null\n");
        return 1;
    }
    if (vertices == NULL)
    {
        fprintf(stderr, "Error: vertices are null\n");
        return 1;
    }
    if (vertex_count == 0)
    {   
        fprintf(stderr, "Error: vertex count is less than 1\n");
        return 1;
    }
    if (indices == NULL)
    {
        fprintf(stderr, "Error: indices are null\n");
        return 1;
    }
    if (index_count == 0)
    {
        fprintf(stderr, "Error: index count is less than 1\n");
        return 1;
    }
    
    GLuint vao = 0;
    GLuint vertex_vbo = 0;
    GLuint ebo = 0;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vertex_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);

    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(indices[0]), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

    glBindVertexArray(0);

    mesh->vao = vao;
    mesh->vertex_vbo = vertex_vbo;
    mesh->vertex_count = (GLsizei)vertex_count;
    mesh->ebo = ebo;
    mesh->index_count = index_count;

    return 0;
}
