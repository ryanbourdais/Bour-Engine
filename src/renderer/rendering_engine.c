#include "rendering_engine.h"
//openGL 4.1 Core
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "window.h"
#include "renderer.h"

static void safe_exit() {
    glfwTerminate();
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void initialize_glfw()
{
    glfwSetErrorCallback(error_callback);
}

static void set_hints()
{
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    // MSAA 8x
    glfwWindowHint( GLFW_SAMPLES, 8 );
}

static void fps_counter(double *delta_time, double *title_countdown_time, GLFWwindow *window)
{
    *title_countdown_time -= *delta_time;
    if (*title_countdown_time <= 0.0 && *delta_time > 0.0)
    {
        double fps = 1.0 / *delta_time;

        // Create a string and put the FPS as the window title.
        char title[256];
        sprintf(title, "FPS = %.2lf", fps);
        glfwSetWindowTitle(window, title);
        *title_countdown_time = 0.1;
    }
}

static void update_frame_time(double current_time, double *previous_time, double *delta_time)
{
    *delta_time = current_time - *previous_time;
    *previous_time = current_time;
}

static void run_engine_loop(GLFWwindow *window, bool fps_enabled)
{
    double previous_time = glfwGetTime();
    double title_countdown_time = 0.1;
    double delta_time = 0.0;

    while (!window_should_close(window))
    {
        double current_time = glfwGetTime();

        update_frame_time(current_time, &previous_time, &delta_time);

        if (fps_enabled)
        {
            fps_counter(&delta_time, &title_countdown_time, window);
        }

        window_poll_events();

        renderer_render_frame(window, delta_time);

        window_present(window);
    }
}

int rendering_engine_entry(bool fullscreen, bool fps_enabled) {
    initialize_glfw();
    if(!glfwInit())
    {
        fprintf(stderr, "GLFW init failed");
        safe_exit();
        return 1;
    }

    set_hints();
    
    GLFWwindow *window = window_create(fullscreen);
    if(window == NULL) {
        safe_exit();
        return 1;
    }

    if (renderer_init(window) != 0)
    {
        fprintf(stderr, "Failed to initialize renderer\n");
        window_destroy(window);
        safe_exit();
        return 1;
    }

    run_engine_loop(window, fps_enabled);

    renderer_shutdown();
    window_destroy(window);
    safe_exit();

    return 0;
}
