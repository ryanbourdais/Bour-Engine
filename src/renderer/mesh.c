#include "mesh.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

//triangle vertices
float points[] = {
    0.0f, 0.5f, 0.0f, //top x,y,z
    0.5f, -0.5f, 0.0f, //bottom right x,y,z
    -0.5f, -0.5f, 0.0f //bottom left x,y,z
};

GLuint create_vbo()
{
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
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