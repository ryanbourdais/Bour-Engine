#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

void render(GLFWwindow* window, bool fps_enabled);
void cleanup(GLuint *vbo, GLuint *vao, GLuint *shader_program);
void Draw(GLFWwindow* window, GLuint *shader_program, GLuint *vao, bool fps_enabled);