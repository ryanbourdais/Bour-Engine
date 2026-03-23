#include <glad/glad.h>

//TODO: Implement Mesh and Mesh Pool struct
/*

typedef struct Mesh {
    GLuint vao;
    GLuint vbo;
    GLsizei vertex_count;
    GLsizei index_count;
} Mesh;

typedef struct MeshPool {
    Mesh* items;
    size_t count;
    size_t capacity;
} MeshPool;

  */
GLuint create_vao(GLuint *vbo);
GLuint create_vbo();
