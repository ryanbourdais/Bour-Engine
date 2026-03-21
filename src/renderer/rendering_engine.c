#include "rendering_engine.h"
//openGL 4.1 Core
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "window.h"

void safe_exit() {
    glfwTerminate();
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void initialize_glfw()
{
    glfwSetErrorCallback(error_callback);
}

void set_hints()
{
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    // MSAA 8x
    glfwWindowHint( GLFW_SAMPLES, 8 );
}

int rendering_engine_entry(bool fullscreen, bool fps_enabled) {
    initialize_glfw();
    bool windowClosed = false;
    if(!glfwInit())
    {
        fprintf(stderr, "GLFW init failed");
    }
    set_hints();
    create_window(&windowClosed, fullscreen, fps_enabled);
    safe_exit();
    return 0;
}