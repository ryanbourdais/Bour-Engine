#include "editor_app.h"
// openGL 4.1 Core
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../renderer/window.h"
#include "../editor/editor_ui.h"
#include "../engine/timing.h"
#include "../utils/profiler.h"
#include "../controller/input.h"
#include "../engine/engine_runtime.h"

#define MAX_EDITOR_HIERARCHY_ITEMS 256

#define ENGINE_SCENE_PATH_MAX_LENGTH 256
#define ENGINE_DEFAULT_SCENE_PATH "test_scene.json"

typedef struct EditorAppFrameProfile {
    ProcessTimer engine_update_timer;
    ProcessTimer scene_extract_timer;
    ProcessTimer editor_begin_timer;
    ProcessTimer renderer_timer;
    ProcessTimer editor_render_timer;
    ProcessTimer present_timer;
} EditorAppFrameProfile;

typedef struct EditorRuntimeFrameScratch {
    EngineRuntimeHierarchyItem runtime_hierarchy_items[
        MAX_EDITOR_HIERARCHY_ITEMS
    ];
    EditorHierarchyItem hierarchy_items[MAX_EDITOR_HIERARCHY_ITEMS];
    EngineRuntimeEntitySnapshot selected_entity;
} EditorRuntimeFrameScratch;

static EditorSelectedLightType editor_app_light_type_from_runtime(
    EngineRuntimeLightType light_type
)
{
    switch (light_type)
    {
        case ENGINE_RUNTIME_LIGHT_DIRECTIONAL:
            return EDITOR_SELECTED_LIGHT_DIRECTIONAL;
        case ENGINE_RUNTIME_LIGHT_POINT:
            return EDITOR_SELECTED_LIGHT_POINT;
        case ENGINE_RUNTIME_LIGHT_SPOT:
            return EDITOR_SELECTED_LIGHT_SPOT;
        default:
            return EDITOR_SELECTED_LIGHT_NONE;
    }
}

static void editor_app_populate_runtime_frame(
    EngineRuntime *runtime,
    uint32_t selected_entity_id,
    EditorFrameData *frame,
    EditorRuntimeFrameScratch *scratch
)
{
    if (runtime == NULL || frame == NULL || scratch == NULL)
    {
        return;
    }

    *scratch = (EditorRuntimeFrameScratch){0};

    size_t hierarchy_count = engine_runtime_copy_hierarchy(
        runtime, 
        scratch->runtime_hierarchy_items, 
        MAX_EDITOR_HIERARCHY_ITEMS
    );

    for (size_t index = 0; index < hierarchy_count; index++)
    {
        scratch->hierarchy_items[index].entity_id =
            scratch->runtime_hierarchy_items[index].entity_id;
        scratch->hierarchy_items[index].name =
            scratch->runtime_hierarchy_items[index].name;
    }

    EngineRuntimeRenderStats render_stats =
        engine_runtime_get_render_stats(runtime);

    
    frame->entity_count = engine_runtime_get_entity_count(runtime);
    frame->renderable_count = render_stats.renderable_count;
    frame->renderer_mesh_count = render_stats.mesh_count;
    frame->renderer_vertex_count = render_stats.vertex_count;
    frame->renderer_triangle_count = render_stats.triangle_count;
    frame->renderer_texture_count = render_stats.texture_count;
    frame->renderer_submitted_draw_count =
        render_stats.submitted_draw_count;
    frame->renderer_submitted_mesh_count =
        render_stats.submitted_mesh_count;
    frame->renderer_submitted_vertex_count =
        render_stats.submitted_vertex_count;
    frame->renderer_submitted_triangle_count =
        render_stats.submitted_triangle_count;
    frame->renderer_missing_model_count =
        render_stats.missing_model_count;
    frame->renderer_viewport_width = render_stats.viewport_width;
    frame->renderer_viewport_height = render_stats.viewport_height;
    frame->renderer_render_target_resize_count =
        render_stats.render_target_resize_count;
    frame->renderer_render_target_noop_count =
        render_stats.render_target_noop_count;
    frame->renderer_zero_size_viewport_count =
        render_stats.zero_size_viewport_count;
    frame->resolved_scene_texture =
        engine_runtime_get_resolved_texture(runtime);
    frame->hierarchy_items = scratch->hierarchy_items;
    frame->hierarchy_item_count = hierarchy_count;

    frame->selected_entity_id = selected_entity_id;
    frame->has_selected_entity = engine_runtime_get_entity_snapshot(
        runtime,
        selected_entity_id,
        &scratch->selected_entity
    );

    
    if (!frame->has_selected_entity)
    {
        frame->selected_entity_name = "No entity selected";
        return;
    }

    frame->selected_entity_name = scratch->selected_entity.name;
    frame->selected_entity_has_transform =
        scratch->selected_entity.has_transform;
    frame->selected_entity_is_renderable =
        scratch->selected_entity.has_mesh_renderer;
    frame->selected_entity_is_programmable_mesh =
        scratch->selected_entity.is_programmable_mesh;
    frame->selected_programmable_mesh_id =
        scratch->selected_entity.programmable_mesh_id;
    frame->selected_programmable_plane_width =
        scratch->selected_entity.programmable_plane_width;
    frame->selected_programmable_plane_depth =
        scratch->selected_entity.programmable_plane_depth;
    frame->selected_programmable_mesh_dirty =
        scratch->selected_entity.programmable_mesh_dirty;

    for (size_t index = 0; index < 3; index++)
    {
        frame->selected_position[index] =
            scratch->selected_entity.position[index];
        frame->selected_rotation[index] =
            scratch->selected_entity.rotation[index];
        frame->selected_scale[index] =
            scratch->selected_entity.scale[index];
        frame->selected_light_ambient[index] =
            scratch->selected_entity.light_ambient[index];
        frame->selected_light_diffuse[index] =
            scratch->selected_entity.light_diffuse[index];
        frame->selected_light_specular[index] =
            scratch->selected_entity.light_specular[index];
        frame->selected_light_direction[index] =
            scratch->selected_entity.light_direction[index];
        frame->selected_light_position[index] =
            scratch->selected_entity.light_position[index];
    }

    frame->selected_light_type = editor_app_light_type_from_runtime(
        scratch->selected_entity.light_type
    );
}

struct EditorAppState
{
    GLFWwindow *window;

    EngineRuntime *runtime;
    float pending_mouse_delta_x;
    float pending_mouse_delta_y;

    EditorAppFrameProfile profile;
    ProcessTimerLogConfig profile_log_config;

    uint32_t selected_entity;
    bool editor_enabled;
    bool editor_cursor_enabled;
    bool editor_scene_view_focused;
    bool editor_camera_capture_active;
    bool fps_enabled;
    FrameClock clock;
    double fps_title_countdown_time;

    bool tab_was_pressed;
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

static double elapsed_timer(double start, double end)
{
    return (end - start) * 1000.0;
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    struct EditorAppState *engine = glfwGetWindowUserPointer(window);

    vec2s offsets = input_get_mouse_offsets(xpos, ypos);

    if (engine->editor_enabled && 
            (engine->editor_cursor_enabled || 
             (!engine->editor_scene_view_focused && !engine->editor_camera_capture_active)))
    {
        return;
    }

    engine->pending_mouse_delta_x = offsets.x;
    engine->pending_mouse_delta_y = offsets.y;
}

static void editor_app_update_runtime(
    struct EditorAppState *engine
)
{
    if (engine == NULL || engine->runtime == NULL)
    {
        return;
    }

    bool camera_input_enabled =
        !engine->editor_enabled ||
        (!engine->editor_cursor_enabled &&
            (engine->editor_scene_view_focused ||
             engine->editor_camera_capture_active));

    vec2s movement_axis = {0};

    if (camera_input_enabled)
    {
        movement_axis = input_get_movement_axis();
    }

    EngineRuntimeInput input = {
        .movement_x = movement_axis.x,
        .movement_y = movement_axis.y,
        .mouse_delta_x = engine->pending_mouse_delta_x,
        .mouse_delta_y = engine->pending_mouse_delta_y,
        .camera_input_enabled = camera_input_enabled,
    };

    engine_runtime_update(
        engine->runtime, 
        frame_clock_delta_time(&engine->clock),
        &input
    );

    engine->pending_mouse_delta_x = 0.0f;
    engine->pending_mouse_delta_y = 0.0f;
}

static void engine_update_editor_cursor_mode(struct EditorAppState *engine)
{
    if (!engine->editor_enabled)
    {
        return;
    }

    bool tab_is_pressed = glfwGetKey(engine->window, GLFW_KEY_TAB) == GLFW_PRESS;

    if (tab_is_pressed && 
            !engine->tab_was_pressed &&
            (!engine->editor_cursor_enabled || engine->editor_scene_view_focused))
    {
        engine->editor_cursor_enabled = !engine->editor_cursor_enabled;

        engine->editor_camera_capture_active = false;

        glfwSetInputMode(engine->window, GLFW_CURSOR, engine->editor_cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
    engine->tab_was_pressed = tab_is_pressed;
}

static bool editor_app_execute_command(
    struct EditorAppState *engine,
    const EngineRuntimeCommand *command,
    uint32_t *out_affected_entity_id
)
{
    if (engine == NULL || engine->runtime == NULL || command == NULL)
    {
        return false;
    }

    EngineRuntimeCommandResult result =
        engine_runtime_execute_command(
            engine->runtime, 
            command, 
            out_affected_entity_id
        );

    if (result != ENGINE_RUNTIME_COMMAND_OK)
    {
        fprintf(
            stderr,
            "Runtime command failed: %d\n",
            result
        );

        return false;
    }

    return true;
}

static void run_editor_app_loop(struct EditorAppState *engine)
{
    while (!window_should_close(engine->window))
    {
        double current_time = glfwGetTime();

        frame_clock_update(&engine->clock, current_time);

        if (engine->fps_enabled)
        {
            fps_counter(&engine->clock.delta_time, &engine->fps_title_countdown_time, engine->window);
        }
        else
        {
            char title[256];
            sprintf(title, "Bour Engine");
            glfwSetWindowTitle(engine->window, title);
        }

        window_poll_events();

        engine_update_editor_cursor_mode(engine);

        double delta_time = frame_clock_delta_time(&engine->clock);

        EditorRuntimeFrameScratch runtime_frame_scratch = {0};

        EditorFrameData editor_frame = {
            .delta_time = delta_time,
            .fps = delta_time > 0.0 ? 1.0 / delta_time : 0.0,
            .editor_cursor_enabled = engine->editor_cursor_enabled,
            .profile_engine_update_ms = 
                engine->profile.engine_update_timer.last_ms,
            .profile_scene_extract_ms =
                engine->profile.scene_extract_timer.last_ms,
            .profile_editor_begin_ms =
                engine->profile.editor_begin_timer.last_ms,
            .profile_renderer_ms =
                engine->profile.renderer_timer.last_ms,
            .profile_editor_render_ms =
                engine->profile.editor_render_timer.last_ms,
            .profile_present_ms =
                engine->profile.present_timer.last_ms,
        };

        process_timer_begin(
            &engine->profile.scene_extract_timer, 
            glfwGetTime()
        );

        editor_app_populate_runtime_frame(
            engine->runtime, 
            engine->selected_entity, 
            &editor_frame, 
            &runtime_frame_scratch
        );

        process_timer_end(
            &engine->profile.scene_extract_timer, 
            glfwGetTime()
        );

        process_timer_log_report(
            &engine->profile.scene_extract_timer, 
            &engine->profile_log_config
        );

        int window_framebuffer_width = 0;
        int window_framebuffer_height = 0;

        window_get_framebuffer_size(
            engine->window,
            &window_framebuffer_width,
            &window_framebuffer_height
        );

        int runtime_framebuffer_width = window_framebuffer_width;
        int runtime_framebuffer_height = window_framebuffer_height;

        if (engine->editor_enabled)
        {
            process_timer_begin(&engine->profile.editor_begin_timer, glfwGetTime());

            EditorFrameResult editor_result = editor_ui_begin_frame(&editor_frame);

            engine->editor_scene_view_focused =
                editor_result.viewport.focused;
            editor_app_update_runtime(engine);

            runtime_framebuffer_width = editor_result.viewport.framebuffer_width;
            runtime_framebuffer_height = editor_result.viewport.framebuffer_height;

            process_timer_end(&engine->profile.editor_begin_timer, glfwGetTime());
            process_timer_log_report(&engine->profile.editor_begin_timer, &engine->profile_log_config);

            uint32_t affected_entity_id = ENGINE_RUNTIME_INVALID_ENTITY_ID;
            EngineRuntimeCommand command = {0};

            if (editor_result.create_empty_entity)
            {
                command.type =
                    ENGINE_RUNTIME_COMMAND_CREATE_EMPTY_ENTITY;

                snprintf(
                    command.name, 
                    ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH, 
                    "%s",
                    "Empty Entity"
                );

                if (editor_app_execute_command(
                        engine, 
                        &command, 
                        &affected_entity_id
                    ))
                {
                    engine->selected_entity = affected_entity_id;
                }
            }

            if (editor_result.create_renderable_entity)
            {
                command = (EngineRuntimeCommand){0};
                command.type =
                    ENGINE_RUNTIME_COMMAND_CREATE_RENDERABLE_ENTITY;

                snprintf(
                    command.name,
                    ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH,
                    "%s",
                    "Renderable Entity"
                );

                if (editor_app_execute_command(
                        engine, 
                        &command, 
                        &affected_entity_id
                    ))
                {
                    engine->selected_entity = affected_entity_id;
                }
            }

            if (editor_result.create_primitive_entity)
            {
                command = (EngineRuntimeCommand){0};
                command.type =
                    ENGINE_RUNTIME_COMMAND_CREATE_PRIMITIVE_ENTITY;

                bool primitive_is_valid = true;

                switch (editor_result.primitive_type_to_create) 
                {
                    case BUILTIN_PRIMITIVE_CUBE:
                        command.primitive_type =
                            ENGINE_RUNTIME_PRIMITIVE_CUBE;
                        break;

                    case BUILTIN_PRIMITIVE_PLANE:
                        command.primitive_type =
                            ENGINE_RUNTIME_PRIMITIVE_PLANE;
                        break;

                    case BUILTIN_PRIMITIVE_QUAD:
                        command.primitive_type =
                            ENGINE_RUNTIME_PRIMITIVE_QUAD;
                        break;

                    case BUILTIN_PRIMITIVE_UV_SPHERE:
                        command.primitive_type =
                            ENGINE_RUNTIME_PRIMITIVE_UV_SPHERE;
                        break;

                    case BUILTIN_PRIMITIVE_CYLINDER:
                        command.primitive_type =
                            ENGINE_RUNTIME_PRIMITIVE_CYLINDER;
                        break;

                    default:
                        command.type =
                            ENGINE_RUNTIME_COMMAND_CREATE_EMPTY_ENTITY;
                        primitive_is_valid = false;
                        break;
                }

                if (primitive_is_valid &&
                    editor_app_execute_command(
                        engine, 
                        &command, 
                        &affected_entity_id
                    ))
                {
                    engine->selected_entity = affected_entity_id;
                }
            }

            if (editor_result.create_programmable_plane)
            {
               command = (EngineRuntimeCommand){0};
               command.type =
                   ENGINE_RUNTIME_COMMAND_CREATE_PROGRAMMABLE_PLANE;
               command.programmable_plane_width = 2.0f;
               command.programmable_plane_depth = 2.0f;

               snprintf(
                    command.name, 
                    ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH, 
                    "%s",
                    "Programmable Plane"
                );
                
               if (editor_app_execute_command(
                        engine,
                        &command,
                        &affected_entity_id
                    ))
                {
                    engine->selected_entity = affected_entity_id;
                }
            }

            if (editor_result.save_scene)
            {
                command = (EngineRuntimeCommand) {
                    .type = ENGINE_RUNTIME_COMMAND_SAVE_SCENE,
                };

                editor_app_execute_command(engine, &command, NULL);
            }

            if (editor_result.load_scene)
            {
                command = (EngineRuntimeCommand) {
                    .type = ENGINE_RUNTIME_COMMAND_LOAD_SCENE,
                };

                if (editor_app_execute_command(engine, &command, NULL))
                {
                    engine->selected_entity = ENGINE_RUNTIME_INVALID_ENTITY_ID;
                }
            }

            if (editor_frame.has_selected_entity)
            {
                if (editor_result.delete_selected_entity)
                {
                    command = (EngineRuntimeCommand) {
                        .type = ENGINE_RUNTIME_COMMAND_DELETE_ENTITY,
                        .entity_id = editor_frame.selected_entity_id,
                    };

                    if (editor_app_execute_command(
                            engine, 
                            &command, 
                            NULL
                        ))
                    {
                        engine->selected_entity = ENGINE_RUNTIME_INVALID_ENTITY_ID;
                    }
                }
                else
                {
                    if (editor_result.duplicate_selected_entity)
                    {
                        command = (EngineRuntimeCommand) {
                            .type = ENGINE_RUNTIME_COMMAND_DUPLICATE_ENTITY,
                            .entity_id = editor_frame.selected_entity_id,
                        };

                        if (editor_app_execute_command(
                                engine, 
                                &command, 
                                &affected_entity_id
                            ))
                        {
                            engine->selected_entity = affected_entity_id;
                        }
                    }

                    if (editor_result.rename_selected_entity)
                    {
                        command = (EngineRuntimeCommand) {
                            .type = ENGINE_RUNTIME_COMMAND_RENAME_ENTITY,
                            .entity_id =
                                editor_frame.selected_entity_id,
                        };

                        snprintf(
                            command.name, 
                            ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH, 
                            "%s",
                            editor_result.edited_name
                        );

                        editor_app_execute_command(
                            engine, 
                            &command, 
                            NULL
                        );
                    }

                    if (editor_result.transform_changed)
                    {
                        command = (EngineRuntimeCommand) {
                            .type = ENGINE_RUNTIME_COMMAND_SET_TRANSFORM,
                            .entity_id = editor_frame.selected_entity_id,
                            .position = {
                                editor_result.edited_position[0],
                                editor_result.edited_position[1],
                                editor_result.edited_position[2],
                            },
                            .rotation = {
                                editor_result.edited_rotation[0],
                                editor_result.edited_rotation[1],
                                editor_result.edited_rotation[2],
                            },
                            .scale = {
                                editor_result.edited_scale[0],
                                editor_result.edited_scale[1],
                                editor_result.edited_scale[2],
                            },
                        };

                        editor_app_execute_command(
                            engine, 
                            &command, 
                            NULL
                        );
                    }

                    if (editor_result.light_changed)
                    {
                        command = (EngineRuntimeCommand) {
                            .type = ENGINE_RUNTIME_COMMAND_SET_LIGHT,
                            .entity_id = editor_frame.selected_entity_id,
                            .light_ambient = {
                                editor_result.edited_light_ambient[0],
                                editor_result.edited_light_ambient[1],
                                editor_result.edited_light_ambient[2],
                            },
                            .light_diffuse = {
                                editor_result.edited_light_diffuse[0],
                                editor_result.edited_light_diffuse[1],
                                editor_result.edited_light_diffuse[2],
                            },
                            .light_specular = {
                                editor_result.edited_light_specular[0],
                                editor_result.edited_light_specular[1],
                                editor_result.edited_light_specular[2],
                            },
                            .light_direction = {
                                editor_result.edited_light_direction[0],
                                editor_result.edited_light_direction[1],
                                editor_result.edited_light_direction[2],
                            },
                            .light_position = {
                                editor_result.edited_light_position[0],
                                editor_result.edited_light_position[1],
                                editor_result.edited_light_position[2],
                            },
                        };

                        editor_app_execute_command(
                            engine, 
                            &command, 
                            NULL
                        );
                    }
                }
            }

            if (editor_result.toggle_editor_cursor)
            {
                engine->editor_cursor_enabled = !engine->editor_cursor_enabled;

                engine->editor_camera_capture_active = !engine->editor_cursor_enabled;
                glfwSetInputMode(engine->window,GLFW_CURSOR,engine->editor_cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            }

            if (editor_result.selection_changed)
            {
                engine->selected_entity = editor_result.selected_entity_id;
            }
        }
        else {
            editor_app_update_runtime(engine);
        }
        process_timer_begin(&engine->profile.renderer_timer, glfwGetTime());

        EngineRuntimeRenderTarget render_target = {
            .framebuffer_width = runtime_framebuffer_width,
            .framebuffer_height = runtime_framebuffer_height,
            .type = engine->editor_enabled
                ? ENGINE_RUNTIME_RENDER_TARGET_OFFSCREEN_TEXTURE
                : ENGINE_RUNTIME_RENDER_TARGET_DEFAULT_FRAMEBUFFER,
        };

        engine_runtime_render(engine->runtime, &render_target);

        process_timer_end(&engine->profile.renderer_timer, glfwGetTime());
        process_timer_log_report(&engine->profile.renderer_timer, &engine->profile_log_config);

        if (engine->editor_enabled)
        {
            process_timer_begin(&engine->profile.editor_render_timer, glfwGetTime());

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, window_framebuffer_width, window_framebuffer_height);
            glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            editor_ui_render();

            process_timer_end(&engine->profile.editor_render_timer, glfwGetTime());
            process_timer_log_report(&engine->profile.editor_render_timer, &engine->profile_log_config);
        }
        process_timer_begin(&engine->profile.present_timer, glfwGetTime());

        window_present(engine->window);
        
        process_timer_end(&engine->profile.present_timer, glfwGetTime());
        process_timer_log_report(&engine->profile.present_timer, &engine->profile_log_config);
    }
}

int editor_app_run(bool fullscreen, bool fps_enabled, bool vsync_enabled)
{
    initialize_glfw();
    if (!glfwInit())
    {
        fprintf(stderr, "GLFW init failed");
        safe_exit();
        return 1;
    }

    set_hints();

    GLFWwindow *window = window_create(fullscreen, vsync_enabled);
    if (window == NULL)
    {
        safe_exit();
        return 1;
    }

    struct EditorAppState engine = {
        .window = window,
        .editor_enabled = true,
        .editor_cursor_enabled = true,
        .editor_camera_capture_active = false,
        .selected_entity = ENGINE_RUNTIME_INVALID_ENTITY_ID,
        .fps_enabled = fps_enabled,
        .fps_title_countdown_time = 0.1,
        .tab_was_pressed = false,
        .profile_log_config = {
            .report_interval_samples = 300,
            .log_average_reports = true,
        },
    };

    glfwSetInputMode(
        engine.window,
        GLFW_CURSOR,
        engine.editor_enabled && engine.editor_cursor_enabled
        ? GLFW_CURSOR_NORMAL
        : GLFW_CURSOR_DISABLED
    );

    frame_clock_init(&engine.clock, glfwGetTime());

    glfwSetWindowUserPointer(window, &engine);
    glfwSetCursorPosCallback(window, mouse_callback);

    int framebuffer_width = 0;
    int framebuffer_height = 0;

    window_get_framebuffer_size(
        engine.window, 
        &framebuffer_width, 
        &framebuffer_height
    );

    EngineRuntimeCreateInfo runtime_create_info = {
        .scene_path = ENGINE_DEFAULT_SCENE_PATH,
        .framebuffer_width = framebuffer_width,
        .framebuffer_height = framebuffer_height,
    };

    engine.runtime = engine_runtime_create(&runtime_create_info);

    if (engine.runtime == NULL)
    {
        fprintf(stderr, "Failed to initialize runtime\n");
        process_timer_log_config_close(&engine.profile_log_config);
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

    process_timer_init(&engine.profile.engine_update_timer, "Engine Update", 16.67, true);
    process_timer_init(&engine.profile.scene_extract_timer, "Scene Extract", 16.67, true);
    process_timer_init(&engine.profile.editor_begin_timer, "Editor Begin", 16.67, true);
    process_timer_init(&engine.profile.renderer_timer, "Renderer", 16.67, true);
    process_timer_init(&engine.profile.editor_render_timer, "Editor Render", 16.67, true);
    process_timer_init(&engine.profile.present_timer, "Present", 100.0, false);

    if (!process_timer_log_config_open(
        &engine.profile_log_config,
        "profile_averages.csv",
        "profile_warnings.csv",
        300,
        true
    ))
    {
        fprintf(stderr, "Failed to open profiler log fields\n");
    }

    run_editor_app_loop(&engine);
    process_timer_log_config_close(&engine.profile_log_config);
    
    engine_runtime_destroy(engine.runtime);

    if (engine.editor_enabled)
    {
        editor_ui_shutdown();
    }
    window_destroy(window);
    safe_exit();

    return 0;
}
