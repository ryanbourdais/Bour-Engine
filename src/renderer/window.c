#include "window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "renderer.h"


void create_window_context(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);
}

void close_window(GLFWwindow* window, bool *windowClosed)
{
    *windowClosed = true;
    glfwDestroyWindow(window);
}

void start_glad()
{
    int version_glad = gladLoadGL();
    if (version_glad == 0)
    {
        fprintf(stderr, "Failed to initialize GLAD");
        exit(-1);
    }
}

double get_time()
{
    double time = glfwGetTime();
    return time;
}

void swap_buffers(GLFWwindow* window)
{
    //vsync
    glfwSwapInterval(1);
    glfwSwapBuffers(window);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        printf("Escape key pressed, closing window\n");
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void create_window(bool *windowClosed, bool fullscreen, bool fps_enabled)
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
        exit(-1);
    }
    create_window_context(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    start_glad();
    render(window, fps_enabled);
    close_window(window, windowClosed);
}
