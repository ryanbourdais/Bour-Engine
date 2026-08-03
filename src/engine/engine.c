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
#include "../editor/editor_ui.h"
#include "timing.h"
#include "../scene/scene.h"

struct EngineState
{
    GLFWwindow *window;
    // Active runtime/editor camera. ECS CameraComponent is scene data for future editor bridging.
    Camera camera;
    Renderer *renderer;
    Scene scene;

    bool editor_enabled;
    bool fps_enabled;
    FrameClock clock;
    double fps_title_countdown_time;
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

static void engine_update_camera(struct EngineState *engine)
{
    vec2s movement_axis = input_get_movement_axis();

    camera_movement(&engine->camera, movement_axis, frame_clock_delta_time(&engine->clock));
    camera_update(&engine->camera);
}

static void engine_update(struct EngineState *engine)
{
    scene_update(&engine->scene, frame_clock_delta_time(&engine->clock));
    engine_update_camera(engine);
}

static void run_engine_loop(struct EngineState *engine)
{
    while (!window_should_close(engine->window))
    {
        double current_time = glfwGetTime();

        frame_clock_update(&engine->clock, current_time);

        if (engine->fps_enabled)
        {
            fps_counter(&engine->clock.delta_time, &engine->fps_title_countdown_time, engine->window);
        }

        window_poll_events();

        engine_update(engine);

        SceneRenderConfig scene_render_config = {0};

        scene_get_render_config(&engine->scene, &scene_render_config);

        RendererFrame frame = {
            .camera = &engine->camera,
            .viewport = {0},
            .renderables = scene_render_config.renderables,
            .renderable_count = scene_render_config.renderable_count,
        };

        window_get_framebuffer_size(engine->window, &frame.viewport.width, &frame.viewport.height);

        if (engine->editor_enabled)
        {
            editor_ui_begin_frame();
        }
        renderer_render_frame(engine->renderer, &frame);

        if (engine->editor_enabled)
        {
            editor_ui_render();
        }
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
        .editor_enabled = true,
        .fps_enabled = fps_enabled,
        .fps_title_countdown_time = 0.1};

    frame_clock_init(&engine.clock, glfwGetTime());

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

    RendererViewport viewport = {0};

    window_get_framebuffer_size(engine.window, &viewport.width, &viewport.height);

    scene_init_default(&engine.scene);

    SceneRenderConfig scene_render_config = {0};

    scene_get_render_config(&engine.scene, &scene_render_config);

    RendererConfig renderer_config = {
        .viewport = viewport,
        .camera = &engine.camera,
        .model_path = scene_render_config.model_path,
        .skybox_faces = {
            scene_render_config.skybox_faces[0],
            scene_render_config.skybox_faces[1],
            scene_render_config.skybox_faces[2],
            scene_render_config.skybox_faces[3],
            scene_render_config.skybox_faces[4],
            scene_render_config.skybox_faces[5],
        },
        .directional_light = scene_render_config.directional_light,
        .point_lights = scene_render_config.point_lights,
        .spot_lights = scene_render_config.spot_lights,
    };

    if (renderer_init(engine.renderer, &renderer_config) != 0)
    {
        fprintf(stderr, "Failed to initialize renderer\n");
        renderer_destroy(engine.renderer);
        scene_shutdown(&engine.scene);
        window_destroy(window);
        safe_exit();
        return 1;
    }
    if (engine.editor_enabled)
    { 
        if (editor_ui_init(engine.window) != 0)
        {
            fprintf(stderr, "Failed to initialize editor UI\n");
            return 1;
        }
    }

    run_engine_loop(&engine);

    renderer_shutdown(engine.renderer);
    renderer_destroy(engine.renderer);
    scene_shutdown(&engine.scene);
    if (engine.editor_enabled)
    {
        editor_ui_shutdown();
    }
    window_destroy(window);
    safe_exit();

    return 0;
}
