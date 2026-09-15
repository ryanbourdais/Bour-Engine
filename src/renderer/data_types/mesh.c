#include "mesh.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

static int mesh_check_gl_errors(const char *operation)
{
    GLenum error = GL_NO_ERROR;
    int failed = 0;

    while ((error = glGetError()) != GL_NO_ERROR)
    {
        fprintf(
            stderr, 
            "OpenGL error after %s: 0x%X\n",
            operation,
            error
        );
        failed = 1;
    }
    return failed;
}

void mesh_init(Mesh *mesh)
{
    if (mesh == NULL)
    {
        return;
    }
    mesh->index_count = 0;
    mesh->vertex_count = 0;
    mesh->texture_count = 0;
    mesh->texture = 0;
    mesh->texture2 = 0;
    mesh->ebo = 0;
    mesh->vao = 0;
    mesh->vertex_vbo = 0;
}

int create_mesh_from_vertices(Mesh *mesh, const Vertex *vertices, size_t vertex_count, const unsigned int *indices, GLsizei index_count)
{
    if (mesh == NULL)
    {
        fprintf(stderr, "Error: mesh is null\n");
        return 1;
    }
    
    mesh_init(mesh);
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
    
    mesh->vao = 0;
    mesh->vertex_vbo = 0;
    mesh->ebo = 0;

    glGenVertexArrays(1, &mesh->vao);

    if (mesh->vao == 0 ||
        mesh_check_gl_errors("Creating mesh vertex array"))
    {
        fprintf(stderr, "Error: failed to create mesh vertex array\n");
        mesh_free(mesh);
        return 1;
    }

    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vertex_vbo);

    if (mesh->vertex_vbo == 0 ||
        mesh_check_gl_errors("Creating mesh vertex buffer"))
    {
        fprintf(stderr, "Error: failed to create mesh vertex buffer\n");
        glBindVertexArray(0);
        mesh_free(mesh);
        return 1;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_vbo);

    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    if (mesh_check_gl_errors("Uploading mesh vertex buffer"))
    {
        fprintf(stderr, "Error: failed to upload mesh vertex buffer\n");
        glBindVertexArray(0);
        mesh_free(mesh);
        return 1;
    }

    glGenBuffers(1, &mesh->ebo);

    if (mesh->ebo == 0 ||
        mesh_check_gl_errors("Creating mesh index buffer"))
    {
        fprintf(stderr, "Error: failed to create mesh index buffer\n");
        glBindVertexArray(0);
        mesh_free(mesh);
        return 1;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(indices[0]), indices, GL_STATIC_DRAW);

    if (mesh_check_gl_errors("Uploading mesh index buffer"))
    {
        fprintf(stderr, "Error: failed to upload mesh index buffer\n");
        glBindVertexArray(0);
        mesh_free(mesh);
        return 1;
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

    mesh->vertex_count = (GLsizei)vertex_count;
    mesh->index_count = index_count;

    return 0;
}

void mesh_free(Mesh *mesh)
{
    if (mesh == NULL)
    {
        return;
    }

    if (mesh->vertex_vbo != 0)
    {
        glDeleteBuffers(1, &mesh->vertex_vbo);
    }

    if (mesh->ebo != 0)
    {
        glDeleteBuffers(1, &mesh->ebo);
    }
    if (mesh->vao != 0)
    {
        glDeleteVertexArrays(1, &mesh->vao);
    }

    mesh_init(mesh);
}

