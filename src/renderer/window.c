#include <glad/glad.h>
#include "window.h"
#include <stdio.h>
#include <stdlib.h>

static void create_window_context(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);
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

GLFWwindow *window_create(bool fullscreen)
{
    GLFWmonitor *mon = NULL;
    int win_w = 1024, win_h = 768; // Our window dimensions, in pixels.

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

    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(        
        win_w,
        win_h,
        "Extended OpenGL Init",
        mon,
        NULL
    );

    if (window == NULL)
    {
        fprintf(stderr, "Window failed to be created\n");
        return NULL;
    }

    create_window_context(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (start_glad() != 0)
    {
        fprintf(stderr, "Failed to start GLAD\n");
        glfwDestroyWindow(window);
        return NULL;
    }

    return window;
}

void window_destroy(GLFWwindow *window)
{
    glfwDestroyWindow(window);
}

bool window_should_close(GLFWwindow *window)
{
    return glfwWindowShouldClose(window);
}

void window_poll_events(void)
{
    glfwPollEvents();
}

void window_present(GLFWwindow *window)
{
    glfwSwapBuffers(window);
}
