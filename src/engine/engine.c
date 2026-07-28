#include "engine.h"
// openGL 4.1 Core
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../renderer/window.h"
#include "../renderer/renderer.h"
#include "../renderer/camera.h"
#include "../controller/input.h"

struct EngineState
{
    GLFWwindow *window;
    Camera camera;
    Renderer *renderer;

    bool fps_enabled;
    double previous_time;
    double delta_time;
    double title_countdown_time;
};

static void safe_exit()
{
    glfwTerminate();
}

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void initialize_glfw()
{
    glfwSetErrorCallback(error_callback);
}

static void set_hints()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // MSAA 8x
    glfwWindowHint(GLFW_SAMPLES, 8);
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

static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    struct EngineState *engine = glfwGetWindowUserPointer(window);

    vec2s offsets = input_get_mouse_offsets(xpos, ypos);

    handle_mouse(&engine->camera, offsets, true);
}

static void update_frame_time(double current_time, double *previous_time, double *delta_time)
{
    *delta_time = current_time - *previous_time;
    *previous_time = current_time;
}

static void engine_update(struct EngineState *engine)
{
    vec2s movement_axis = input_get_movement_axis();

    camera_movement(
        &engine->camera,
        movement_axis,
        engine->delta_time);

    camera_update(&engine->camera);
}

static void run_engine_loop(struct EngineState *engine)
{
    while (!window_should_close(engine->window))
    {
        double current_time = glfwGetTime();

        update_frame_time(current_time, &engine->previous_time, &engine->delta_time);

        if (engine->fps_enabled)
        {
            fps_counter(&engine->delta_time, &engine->title_countdown_time, engine->window);
        }

        window_poll_events();

        engine_update(engine);

        RendererFrame frame = {.camera = &engine->camera, .framebuffer_width = 0, .framebuffer_height = 0};

        window_get_framebuffer_size(engine->window, &frame.framebuffer_width, &frame.framebuffer_height);

        renderer_render_frame(engine->renderer, &frame);

        window_present(engine->window);
    }
}

int engine_run(bool fullscreen, bool fps_enabled)
{
    initialize_glfw();
    if (!glfwInit())
    {
        fprintf(stderr, "GLFW init failed");
        safe_exit();
        return 1;
    }

    set_hints();

    GLFWwindow *window = window_create(fullscreen);
    if (window == NULL)
    {
        safe_exit();
        return 1;
    }

    struct EngineState engine = {
        .window = window,
        .fps_enabled = fps_enabled,
        .previous_time = glfwGetTime(),
        .delta_time = 0.0,
        .title_countdown_time = 0.1};

    glfwSetWindowUserPointer(window, &engine);
    glfwSetCursorPosCallback(window, mouse_callback);

    engine.renderer = renderer_create();

    if (engine.renderer == NULL)
    {
        window_destroy(window);
        safe_exit();
        return 1;
    }

    camera_init(&engine.camera);
    camera_update(&engine.camera);

    if (renderer_init(engine.renderer, window, &engine.camera) != 0)
    {
        fprintf(stderr, "Failed to initialize renderer\n");
        renderer_destroy(engine.renderer);
        window_destroy(window);
        safe_exit();
        return 1;
    }

    run_engine_loop(&engine);

    renderer_shutdown(engine.renderer);
    renderer_destroy(engine.renderer);
    window_destroy(window);
    safe_exit();

    return 0;
}
