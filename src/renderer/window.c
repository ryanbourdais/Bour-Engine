#include "window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include "renderer.h"

static void create_window_context(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);
}

static void close_window(GLFWwindow* window)
{
    glfwDestroyWindow(window);
}

static int start_glad()
{
    int version_glad = gladLoadGL();
    if (version_glad == 0)
    {
        fprintf(stderr, "Failed to initialize GLAD");
        return 1;
    }
    return 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        printf("Escape key pressed, closing window\n");
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int window_run( bool fullscreen, bool fps_enabled)
{
    GLFWmonitor *mon = NULL;
    int win_w = 800, win_h = 600; // Our window dimensions, in pixels.

    if ( fullscreen ) {
        mon = glfwGetPrimaryMonitor();

        const GLFWvidmode* mode = glfwGetVideoMode( mon );

        // Hinting these properties lets us use "borderless full screen" mode.
        glfwWindowHint( GLFW_RED_BITS, mode->redBits );
        glfwWindowHint( GLFW_GREEN_BITS, mode->greenBits );
        glfwWindowHint( GLFW_BLUE_BITS, mode->blueBits );
        glfwWindowHint( GLFW_REFRESH_RATE, mode->refreshRate );

        win_w = mode->width;  // Use our 'desktop' resolution for window size
        win_h = mode->height; // to get a 'full screen borderless' window.
    }

    GLFWwindow* window = glfwCreateWindow(
        win_w,
        win_h,
        "Extended OpenGL Init",
        mon,
        NULL
    );
    if(!window)
    {
        fprintf(stderr, "Window failed to be created");
        return 1;
    }
    create_window_context(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    int glad_result = start_glad();
    if(glad_result != 0) {
        fprintf(stderr, "Failed to start GLAD");
        close_window(window);
        return 1;
    }
    int renderer_result = renderer_run(window, fps_enabled);
    if(renderer_result != 0) {
        fprintf(stderr, "Failed to run renderer\n");
        close_window(window);
        return 1;
    }
    close_window(window);
    return 0;
}
