#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "shaders.h"
#include "mesh.h"



void fps_counter(double *previous_time, double *title_countdown_time, GLFWwindow* window)
{
    double current_time = glfwGetTime();
    double elapsed_time = current_time - *previous_time;
    *previous_time = current_time;

    *title_countdown_time -= elapsed_time;
    if ( *title_countdown_time <= 0.0 && elapsed_time > 0.0 ) {
        double fps = 1.0 / elapsed_time;

        // Create a string and put the FPS as the window title.
        char title[256];
        sprintf(title, "FPS = %.2lf", fps);
        glfwSetWindowTitle(window, title);
        *title_countdown_time = 0.1;
    }
}

void Draw(GLFWwindow* window, GLuint *shader_program, GLuint *vao, bool fps_enabled)
{

    double previous_time = glfwGetTime();
    double title_countdown_time = 0.1;
    while(!glfwWindowShouldClose(window))
    {
        if(fps_enabled)
        {
            fps_counter(&previous_time, &title_countdown_time, window);
        }
        // Update window events
        glfwPollEvents();

        // Wipe drawing surface clear
        glClearColor( 0.6f, 0.6f, 0.8f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Put the shader program and VAO in focus in OpenGL's state machine
        glUseProgram(*shader_program);
        glBindVertexArray(*vao);

        // Draw points 0-3 from currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Put the drawing into the visible area
        glfwSwapBuffers(window);
    }
}

void cleanup(GLuint *vbo, GLuint *vao, GLuint *shader_program)
{
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, vao);
    glDeleteProgram(*shader_program);
}

void render(GLFWwindow* window, bool fps_enabled)
{
    GLuint vbo = create_vbo();
    GLuint vao = create_vao(&vbo);
    GLuint vs, fs;
    load_shaders(&vs, &fs);
    GLuint shader_program;
    create_shader_program(&vs, &fs, &shader_program);
    Draw(window, &shader_program, &vao, fps_enabled);
    cleanup(&vbo, &vao, &shader_program);
}