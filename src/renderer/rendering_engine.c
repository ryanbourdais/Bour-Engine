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

    int renderer_result = renderer_run(window, fps_enabled);

    window_destroy(window);

    if( renderer_result != 0) {
        fprintf(stderr, "Failed to run renderer\n");
        safe_exit();
        return 1;
    }

    safe_exit();
    return 0;
}
