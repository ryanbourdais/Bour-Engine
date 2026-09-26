#include <GLFW/glfw3.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../engine/engine_runtime.h"
#include "../renderer/window.h"

typedef struct GameApplication {
    GLFWwindow *window;
    EngineRuntime *runtime;
    EngineRuntimeInput input;
    bool has_mouse_position;
    double last_mouse_x;
    double last_mouse_y;
} GameApplication;

static void glfw_error_callback(
    int error,
    const char *description
)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

static void configure_window_hints(void)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);
}

static void mouse_callback(
    GLFWwindow *window,
    double mouse_x,
    double mouse_y
)
{
    GameApplication *application =
        glfwGetWindowUserPointer(window);

    if (application == NULL)
    {
        return;
    }

    if (!application->has_mouse_position)
    {
        application->last_mouse_x = mouse_x;
        application->last_mouse_y = mouse_y;
        application->has_mouse_position = true;
        return;
    }

    application->input.mouse_delta_x +=
        (float)(mouse_x - application->last_mouse_x);
    application->input.mouse_delta_y +=
        (float)(application->last_mouse_y - mouse_y);

    application->last_mouse_x = mouse_x;
    application->last_mouse_y = mouse_y;
}

static void read_game_input(GameApplication *application)
{
    GLFWwindow *window = application->window;

    application->input.movement_x =
        (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? 1.0f : 0.0f) -
        (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? 1.0f : 0.0f);

    application->input.movement_y =
        (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? 1.0f : 0.0f) -
        (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? 1.0f : 0.0f);

    application->input.camera_input_enabled = true;

    application->input.simulation_update_enabled = true;
}

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        fprintf(stderr, "Usage: bour_game [scene-path]\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }

    configure_window_hints();

    GLFWwindow *window = window_create(false, true);

    if (window == NULL)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    int framebuffer_width = 0;
    int framebuffer_height = 0;

    window_get_framebuffer_size(
        window, 
        &framebuffer_width, 
        &framebuffer_height
    );

    EngineRuntimeCreateInfo create_info = {
        .scene_path = argc == 2 ? argv[1] : NULL,
        .framebuffer_width = framebuffer_width,
        .framebuffer_height = framebuffer_height,
    };

    EngineRuntime *runtime = engine_runtime_create(&create_info);

    if (runtime == NULL)
    {
        fprintf(stderr, "Failed to create runtime\n");
        window_destroy(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    GameApplication application = {
        .window = window,
        .runtime = runtime,
    };

    glfwSetWindowUserPointer(window, &application);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    double previous_time = glfwGetTime();

    while (!window_should_close(window))
    {
        double current_time = glfwGetTime();
        double delta_time = current_time - previous_time;
        previous_time = current_time;

        window_poll_events();
        read_game_input(&application);

        engine_runtime_update(
            application.runtime, 
            delta_time, 
            &application.input
        );

        application.input.mouse_delta_x = 0.0f;
        application.input.mouse_delta_y = 0.0f;

        window_get_framebuffer_size(
            application.window, 
            &framebuffer_width, 
            &framebuffer_height
        );

        EngineRuntimeRenderTarget render_target = {
            .framebuffer_width = framebuffer_width,
            .framebuffer_height = framebuffer_height,
            .type =
                ENGINE_RUNTIME_RENDER_TARGET_DEFAULT_FRAMEBUFFER,
        };

        engine_runtime_render(application.runtime, &render_target);
        window_present(application.window);
    }

    engine_runtime_destroy(application.runtime);
    window_destroy(application.window);
    glfwTerminate();

    return EXIT_SUCCESS;
}



