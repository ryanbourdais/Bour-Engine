#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include "shaders.h"
#include "mesh.h"

struct RendererState {
    GLuint shader_program;
    GLuint vao;
    GLuint vbo;
};

static void fps_counter(double *previous_time, double *title_countdown_time, GLFWwindow* window)
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

static void run_render_loop(GLFWwindow* window, GLuint *shader_program, GLuint *vao, bool fps_enabled)
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

static int renderer_init(struct RendererState *renderer)
{
    GLuint vs, fs;

    renderer->vbo = create_vbo();
    renderer->vao = create_vao(&renderer->vbo);

    if (load_shaders(&vs, &fs) != 0) {
        return 1;
    }

    if (create_shader_program(&vs, &fs, &renderer->shader_program) != 0) {
        return 1;
    }

    return 0;
}

static void renderer_shutdown(struct RendererState *renderer)
{
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteProgram(renderer->shader_program);
}

int renderer_run(GLFWwindow* window, bool fps_enabled)
{
    struct RendererState renderer = {0};

    if (renderer_init(&renderer) != 0) {
        renderer_shutdown(&renderer);
        fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }
    run_render_loop(window, &renderer.shader_program, &renderer.vao, fps_enabled);
    renderer_shutdown(&renderer);
    return 0;
}