#include "engine.h"
// openGL 4.1 Core
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../renderer/window.h"
#include "../renderer/renderer.h"
#include "../renderer/camera.h"
#include "../controller/input.h"
#include "../editor/editor_ui.h"
#include "../scene/scene.h"
#include "timing.h"
#include "../utils/math_utils.h"
#include "../utils/profiler.h"
#include "../scene/scene_serialization.h"

#define MAX_EDITOR_HIERARCHY_ITEMS 256

typedef struct EngineFrameProfile {
    ProcessTimer engine_update_timer;
    ProcessTimer scene_extract_timer;
    ProcessTimer editor_begin_timer;
    ProcessTimer renderer_timer;
    ProcessTimer editor_render_timer;
    ProcessTimer present_timer;
} EngineFrameProfile;

struct EngineState
{
    GLFWwindow *window;
    // Active runtime/editor camera. ECS CameraComponent is scene data for future editor bridging.
    Camera camera;
    Renderer *renderer;
    Scene scene;

    EngineFrameProfile profile;
    ProcessTimerLogConfig profile_log_config;

    EntityId selected_entity;
    bool editor_enabled;
    bool editor_cursor_enabled;
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

static bool engine_entity_is_valid(struct EngineState *engine, EntityId entity)
{
    return entity_registry_is_alive(&engine->scene.entities, entity);
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
    struct EngineState *engine = glfwGetWindowUserPointer(window);

    vec2s offsets = input_get_mouse_offsets(xpos, ypos);

    if (engine->editor_enabled && engine->editor_cursor_enabled)
    {
        return;
    }

    handle_mouse(&engine->camera, offsets, true);
}

static void engine_update_camera(struct EngineState *engine)
{
    vec2s movement_axis = input_get_movement_axis();

    camera_movement(&engine->camera, movement_axis, frame_clock_delta_time(&engine->clock));
    camera_update(&engine->camera);
}

static void engine_update_editor_cursor_mode(struct EngineState *engine)
{
    if (!engine->editor_enabled)
    {
        return;
    }

    bool tab_is_pressed = glfwGetKey(engine->window, GLFW_KEY_TAB) == GLFW_PRESS;

    if (tab_is_pressed && !engine->tab_was_pressed)
    {
        engine->editor_cursor_enabled = !engine->editor_cursor_enabled;

        glfwSetInputMode(engine->window, GLFW_CURSOR, engine->editor_cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
    engine->tab_was_pressed = tab_is_pressed;
}

static void engine_update(struct EngineState *engine)
{
    scene_update(&engine->scene, frame_clock_delta_time(&engine->clock));
    engine_update_editor_cursor_mode(engine);
    engine_update_camera(engine);
}

static EntityId engine_create_empty_entity(struct EngineState *engine, const char *name_value)
{
    EntityId entity = entity_registry_create(&engine->scene.entities);

    NameComponent name = {0};
    snprintf(name.value, ENTITY_NAME_MAX_LENGTH, "%s", name_value);

    TransformComponent transform;
    transform_component_init(&transform);

    component_storage_add(&engine->scene.names, entity, &name);
    component_storage_add(&engine->scene.transforms, entity, &transform);

    return entity;
}

static EntityId engine_create_renderable_entity(struct EngineState *engine, const char *name_value)
{
    EntityId entity = engine_create_empty_entity(engine, name_value);

    MeshRendererComponent mesh_renderer = {
        .model_path = engine->scene.model_path,
    };

    component_storage_add(&engine->scene.mesh_renderers, entity, &mesh_renderer);

    return entity;
}

static bool engine_get_selected_transform(
    struct EngineState *engine,
    EntityId selected_entity,
    float out_position[3],
    float out_rotation[3],
    float out_scale[3]
)
{
    if (!engine_entity_is_valid(engine, selected_entity))
    {
        return false;
    }
    const TransformComponent *selected_transform = 
        (const TransformComponent *)component_storage_get(&engine->scene.transforms, selected_entity);

    if (selected_transform != NULL)
    {
        copy_vec3_xyz_to_float3(out_position, selected_transform->position);
        copy_vec3_xyz_to_float3(out_rotation, selected_transform->rotation);
        copy_vec3_xyz_to_float3(out_scale, selected_transform->scale);
        return true;
    }
    return false;
}

static EditorSelectedLightType engine_get_selected_light(
    struct EngineState *engine,
    EntityId selected_entity,
    float out_ambient[3], 
    float out_diffuse[3],
    float out_specular[3],
    float out_direction[3],
    float out_position[3]
)
{
    if (!engine_entity_is_valid(engine, selected_entity))
    {
        return EDITOR_SELECTED_LIGHT_NONE;
    }
    DirectionalLightComponent *directional = component_storage_get(&engine->scene.directional_lights, selected_entity);
    PointLightComponent *point = component_storage_get(&engine->scene.point_lights, selected_entity);
    SpotLightComponent *spot = component_storage_get(&engine->scene.spot_lights, selected_entity);

    EditorSelectedLightType selected_light_type = EDITOR_SELECTED_LIGHT_NONE;

    if (directional != NULL)
    {
        selected_light_type = EDITOR_SELECTED_LIGHT_DIRECTIONAL;
        copy_vec3_rgb_to_float3(out_ambient, directional->light.color.ambient);
        copy_vec3_rgb_to_float3(out_diffuse, directional->light.color.diffuse);
        copy_vec3_rgb_to_float3(out_specular, directional->light.color.specular);
        copy_vec3_xyz_to_float3(out_direction, directional->light.direction);
    }
    else if (point != NULL)
    {
        selected_light_type = EDITOR_SELECTED_LIGHT_POINT;
        copy_vec3_rgb_to_float3(out_ambient, point->light.color.ambient);
        copy_vec3_rgb_to_float3(out_diffuse, point->light.color.diffuse);
        copy_vec3_rgb_to_float3(out_specular, point->light.color.specular);
        copy_vec3_xyz_to_float3(out_position, point->light.position);
    }
    else if (spot != NULL)
    {
        selected_light_type = EDITOR_SELECTED_LIGHT_SPOT;
        copy_vec3_rgb_to_float3(out_ambient, spot->light.color.ambient);
        copy_vec3_rgb_to_float3(out_diffuse, spot->light.color.diffuse);
        copy_vec3_rgb_to_float3(out_specular, spot->light.color.specular);
        copy_vec3_xyz_to_float3(out_direction, spot->light.direction);
        copy_vec3_xyz_to_float3(out_position, spot->light.position);
    }
    return selected_light_type;
}

static void engine_delete_selected_entity(struct EngineState *engine, EntityId selected_entity)
{
    if (!engine_entity_is_valid(engine, selected_entity))
    {
        return;
    }
    component_storage_remove(&engine->scene.names, selected_entity);
    component_storage_remove(&engine->scene.transforms, selected_entity);
    component_storage_remove(&engine->scene.mesh_renderers, selected_entity);
    component_storage_remove(&engine->scene.directional_lights, selected_entity);
    component_storage_remove(&engine->scene.spot_lights, selected_entity);
    component_storage_remove(&engine->scene.point_lights, selected_entity);
    component_storage_remove(&engine->scene.cameras, selected_entity);
                
    entity_registry_destroy(&engine->scene.entities, selected_entity);

    engine->selected_entity = INVALID_ENTITY_ID;
}

static void engine_duplicate_selected_entity(struct EngineState *engine, EntityId selected_entity)
{
    if (!engine_entity_is_valid(engine, selected_entity))
    {
        return;
    }
    TransformComponent *source_transform = (TransformComponent *)component_storage_get(&engine->scene.transforms, selected_entity);
                        
    MeshRendererComponent *source_mesh = (MeshRendererComponent *)component_storage_get(&engine->scene.mesh_renderers, selected_entity);

    if (source_transform != NULL && source_mesh != NULL)
    {
        EntityId duplicate = engine_create_empty_entity(engine, "Duplicated Entity");

        TransformComponent *duplicate_transform = (TransformComponent *)component_storage_get(&engine->scene.transforms, duplicate);

        if (duplicate_transform != NULL)
        {
            *duplicate_transform = *source_transform;
            duplicate_transform->position.x += 1.0f;
        }

        MeshRendererComponent mesh_renderer = {
            .model_path = source_mesh->model_path,
        };

        component_storage_add(&engine->scene.mesh_renderers, duplicate, &mesh_renderer);

        engine->selected_entity = duplicate;
    }
}

static void engine_rename_selected_entity(struct EngineState *engine, EntityId selected_entity, char* edited_name)
{
    if (!engine_entity_is_valid(engine, selected_entity))
    {
        return;
    }
    NameComponent *name = (NameComponent *)component_storage_get(&engine->scene.names, selected_entity);
    if (name == NULL)
    {
        NameComponent new_name = {0};
        snprintf(new_name.value, ENTITY_NAME_MAX_LENGTH, "%s", edited_name);
        component_storage_add(&engine->scene.names, selected_entity, &new_name);
    }
    else
    {
        snprintf(name->value, ENTITY_NAME_MAX_LENGTH, "%s", edited_name);
    }
}

static void engine_change_selected_transform(
    struct EngineState *engine,
    EntityId selected_entity,
    float* new_position,
    float* new_rotation,
    float* new_scale
)
{
    if (!engine_entity_is_valid(engine, selected_entity))
    {
        return;
    }
    TransformComponent *transform = (TransformComponent *)component_storage_get(&engine->scene.transforms, selected_entity);
    if (transform != NULL)
    {
        vec3s new_position_vec;
        copy_float3_to_vec3_xyz(&new_position_vec, new_position);
        transform_component_set_position(
            transform,
            new_position_vec
        );
        vec3s new_rotation_vec;
        copy_float3_to_vec3_xyz(&new_rotation_vec, new_rotation);
        transform_component_set_rotation(
            transform,
            new_rotation_vec
        );
        vec3s new_scale_vec;
        copy_float3_to_vec3_xyz(&new_scale_vec, new_scale);
        transform_component_set_scale(
            transform,
            new_scale_vec
        );
    }
}

static void engine_modify_selected_light(
    struct EngineState *engine,
    EntityId selected_entity,
    float* edited_ambient,
    float* edited_diffuse,
    float* edited_specular,
    float* edited_direction,
    float* edited_position
)
{
    if (!engine_entity_is_valid(engine, selected_entity))
    {
        return;
    }
    DirectionalLightComponent *directional = component_storage_get(&engine->scene.directional_lights, selected_entity);
    PointLightComponent *point = component_storage_get(&engine->scene.point_lights, selected_entity);
    SpotLightComponent *spot = component_storage_get(&engine->scene.spot_lights, selected_entity);

    vec3s ambient = {{
        edited_ambient[0],
        edited_ambient[1],
        edited_ambient[2]
    }};
    vec3s diffuse = {{
        edited_diffuse[0],
        edited_diffuse[1],
        edited_diffuse[2]
    }};
    vec3s specular = {{
        edited_specular[0],
        edited_specular[1],
        edited_specular[2]
    }};
    vec3s direction = {0};
    vec3s position = {0};

    direction = (vec3s){{
        edited_direction[0],
        edited_direction[1],
        edited_direction[2],
    }};
    position = (vec3s){{
        edited_position[0],
        edited_position[1],
        edited_position[2],
    }};
    
    if (directional != NULL)
    {
        directional->light.color.ambient = ambient;
        directional->light.color.diffuse = diffuse;
        directional->light.color.specular = specular;
        directional->light.direction = direction;
    }
    else if (point != NULL)
    {
        point->light.color.ambient = ambient;
        point->light.color.diffuse = diffuse;
        point->light.color.specular = specular;
        point->light.position = position;
    }
    else if (spot != NULL)
    {
        spot->light.color.ambient = ambient;
        spot->light.color.diffuse = diffuse;
        spot->light.color.specular = specular;
        spot->light.direction = direction;
        spot->light.position = position;
    }
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
        else {
            char title[256];
            sprintf(title, "Bour Engine");
            glfwSetWindowTitle(engine->window, title);
        }

        window_poll_events();

        process_timer_begin(&engine->profile.engine_update_timer, glfwGetTime());
        engine_update(engine);
        process_timer_end(&engine->profile.engine_update_timer, glfwGetTime());
        process_timer_log_report(&engine->profile.engine_update_timer, &engine->profile_log_config);

        SceneRenderConfig scene_render_config = {0};

        process_timer_begin(&engine->profile.scene_extract_timer, glfwGetTime()); 
        scene_get_render_config(&engine->scene, &scene_render_config);
        process_timer_end(&engine->profile.scene_extract_timer, glfwGetTime());
        process_timer_log_report(&engine->profile.scene_extract_timer, &engine->profile_log_config);

        size_t hierarchy_count = engine->scene.entities.count;
        if (hierarchy_count > MAX_EDITOR_HIERARCHY_ITEMS)
        {
            hierarchy_count = (size_t)MAX_EDITOR_HIERARCHY_ITEMS;
        }

        EditorHierarchyItem hierarchy_items[MAX_EDITOR_HIERARCHY_ITEMS];

        for(size_t i = 0; i < hierarchy_count; i++)
        {
            EntityId entity = engine->scene.entities.entities[i];
            hierarchy_items[i].entity_id = entity;

            const NameComponent *name = (const NameComponent *)component_storage_get(&engine->scene.names, entity);

            if (name == NULL)
            {
                hierarchy_items[i].name = "Unnamed Entity";
            }
            else {
                hierarchy_items[i].name = name->value;
            }
        }

        double delta_time = frame_clock_delta_time(&engine->clock);

        bool has_selected_entity = entity_registry_is_alive(&engine->scene.entities, engine->selected_entity);

        const char *selected_entity_name = "No entity selected";
        bool selected_entity_has_transform = false;

        float selected_position[3] = {0};
        float selected_rotation[3] = {0};
        float selected_scale[3] = {0};

        bool selected_entity_is_renderable = false;

        if (has_selected_entity)
        {
            const NameComponent *selected_name = (const NameComponent *)component_storage_get(&engine->scene.names, engine->selected_entity);
            selected_entity_name = selected_name != NULL ? selected_name->value : "Unnamed Entity";

            selected_entity_is_renderable = component_storage_get(&engine->scene.mesh_renderers, engine->selected_entity) != NULL;

            selected_entity_has_transform = engine_get_selected_transform(
                engine, engine->selected_entity,
                selected_position,
                selected_rotation,
                selected_scale
            );
        }

        float selected_light_ambient[3] = {0};
        float selected_light_diffuse[3] = {0};
        float selected_light_specular[3] = {0};
        float selected_light_direction[3] = {0};
        float selected_light_position[3] = {0};

        EditorSelectedLightType selected_light_type = engine_get_selected_light(
            engine, engine->selected_entity,
            selected_light_ambient,
            selected_light_diffuse,
            selected_light_specular,
            selected_light_direction,
            selected_light_position
        );



        RendererFrame frame = {
            .camera = &engine->camera,
            .viewport = {0},
            .renderables = scene_render_config.renderables,
            .renderable_count = scene_render_config.renderable_count,
            .directional_light = scene_render_config.directional_light,
            .spot_lights = scene_render_config.spot_lights,
            .point_lights = scene_render_config.point_lights,
        };

        RendererStats renderer_stats = renderer_get_frame_stats(engine->renderer, &frame);

        EditorFrameData editor_frame = {
            .delta_time = delta_time,
            .fps = delta_time > 0.0 ? 1.0 / delta_time : 0.0,
            .entity_count = engine->scene.entities.count,
            .selected_entity_id = engine->selected_entity,
            .has_selected_entity = has_selected_entity,
            .selected_entity_has_transform = selected_entity_has_transform,
            .selected_entity_name = selected_entity_name,
            .selected_position = { selected_position[0], selected_position[1], selected_position[2] },
            .selected_rotation = { selected_rotation[0], selected_rotation[1], selected_rotation[2] },
            .selected_scale = { selected_scale[0], selected_scale[1], selected_scale[2] },
            .selected_light_ambient = {selected_light_ambient[0],selected_light_ambient[1], selected_light_ambient[2]},
            .selected_light_diffuse = {selected_light_diffuse[0],selected_light_diffuse[1], selected_light_diffuse[2]},
            .selected_light_direction = {selected_light_direction[0],selected_light_direction[1], selected_light_direction[2]},
            .selected_light_position = {selected_light_position[0],selected_light_position[1], selected_light_position[2]},
            .selected_light_specular = {selected_light_specular[0],selected_light_specular[1], selected_light_specular[2]},
            .selected_light_type = selected_light_type,
            .selected_entity_is_renderable = selected_entity_is_renderable,
            .renderable_count = scene_render_config.renderable_count,
            .renderer_mesh_count = renderer_stats.mesh_count,
            .renderer_vertex_count = renderer_stats.vertex_count,
            .renderer_triangle_count = renderer_stats.triangle_count,
            .renderer_texture_count = renderer_stats.texture_count,
            .renderer_submitted_draw_count = renderer_stats.submitted_draw_count,
            .renderer_submitted_mesh_count = renderer_stats.submitted_mesh_count,
            .renderer_submitted_vertex_count = renderer_stats.submitted_vertex_count,
            .renderer_submitted_triangle_count = renderer_stats.submitted_triangle_count,
            .renderer_missing_model_count = renderer_stats.missing_model_count,
            .editor_cursor_enabled = engine->editor_cursor_enabled,
            .profile_engine_update_ms = engine->profile.engine_update_timer.last_ms,
            .profile_scene_extract_ms = engine->profile.scene_extract_timer.last_ms,
            .profile_editor_begin_ms = engine->profile.editor_begin_timer.last_ms,
            .profile_renderer_ms = engine->profile.renderer_timer.last_ms,
            .profile_editor_render_ms = engine->profile.editor_render_timer.last_ms,
            .profile_present_ms = engine->profile.present_timer.last_ms,
            .hierarchy_items = hierarchy_items,
            .hierarchy_item_count = hierarchy_count,
        };

        window_get_framebuffer_size(engine->window, &frame.viewport.width, &frame.viewport.height);

        if (engine->editor_enabled)
        {
            process_timer_begin(&engine->profile.editor_begin_timer, glfwGetTime());

            EditorFrameResult editor_result = editor_ui_begin_frame(&editor_frame);

            process_timer_end(&engine->profile.editor_begin_timer, glfwGetTime());
            process_timer_log_report(&engine->profile.editor_begin_timer, &engine->profile_log_config);

            EntityId result_entity = editor_frame.selected_entity_id;

            bool result_entity_alive = entity_registry_is_alive(&engine->scene.entities, result_entity);

            if (editor_result.create_empty_entity)
            {
                engine->selected_entity = engine_create_empty_entity(engine, "Empty Entity");
            }

            if (editor_result.create_renderable_entity)
            {
                engine->selected_entity = engine_create_renderable_entity(engine, "Renderable Entity");
            }
            if (editor_result.save_scene)
            {
                SceneSaveResult save_result = scene_save_to_file(&engine->scene, "test_scene.json");
                if (save_result != SCENE_SAVE_OK)
                {
                    fprintf(stderr, "Failed to save scene: %d\n", save_result);
                }
            }
            if (editor_result.load_scene)
            {
                SceneLoadResult load_result = scene_load_from_file(&engine->scene, "test_scene.json");
                if (load_result != SCENE_LOAD_OK)
                {
                    fprintf(stderr, "Failed to load scene: %d\n", load_result);
                }
                else {
                    engine->selected_entity = INVALID_ENTITY_ID;
                }
            }

            if (result_entity_alive)
            {
                if (editor_result.delete_selected_entity)
                {
                    engine_delete_selected_entity(engine, result_entity);
                }
                else {
                    if (editor_result.duplicate_selected_entity)
                    {
                        engine_duplicate_selected_entity(engine, result_entity);
                    }

                    if (editor_result.rename_selected_entity)
                    {
                        engine_rename_selected_entity(engine, result_entity, editor_result.edited_name);
                    }

                    if (editor_result.transform_changed)
                    {
                        engine_change_selected_transform(engine, result_entity, editor_result.edited_position, editor_result.edited_rotation, editor_result.edited_scale);
                    }
                    if (editor_result.light_changed)
                    {
                        engine_modify_selected_light(
                            engine,
                            result_entity,
                            editor_result.edited_light_ambient,
                            editor_result.edited_light_diffuse,
                            editor_result.edited_light_specular,
                            editor_result.edited_light_direction,
                            editor_result.edited_light_position
                        );
                    }
                }
            }

            if (editor_result.toggle_editor_cursor)
            {
                engine->editor_cursor_enabled = !engine->editor_cursor_enabled;

                glfwSetInputMode(engine->window,GLFW_CURSOR,engine->editor_cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            }

            if (editor_result.selection_changed)
            {
                engine->selected_entity = editor_result.selected_entity_id;
            }
        }
        process_timer_begin(&engine->profile.renderer_timer, glfwGetTime());

        renderer_render_frame(engine->renderer, &frame);

        process_timer_end(&engine->profile.renderer_timer, glfwGetTime());
        process_timer_log_report(&engine->profile.renderer_timer, &engine->profile_log_config);

        if (engine->editor_enabled)
        {
            process_timer_begin(&engine->profile.editor_render_timer, glfwGetTime());

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

int engine_run(bool fullscreen, bool fps_enabled, bool vsync_enabled)
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

    struct EngineState engine = {
        .window = window,
        .editor_enabled = true,
        .editor_cursor_enabled = true,
        .selected_entity = INVALID_ENTITY_ID,
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
        engine.editor_cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED
    );

    frame_clock_init(&engine.clock, glfwGetTime());

    glfwSetWindowUserPointer(window, &engine);
    glfwSetCursorPosCallback(window, mouse_callback);

    engine.renderer = renderer_create();

    if (engine.renderer == NULL)
    {
        process_timer_log_config_close(&engine.profile_log_config);
        window_destroy(window);
        safe_exit();
        return 1;
    }

    camera_init(&engine.camera);
    camera_update(&engine.camera);

    RendererViewport viewport = {0};

    window_get_framebuffer_size(engine.window, &viewport.width, &viewport.height);

    scene_init_default(&engine.scene);


    // Debug trigger for saving on first load
    // =======================================
    // SceneSaveResult save_result = scene_save_to_file(&engine.scene, "test_scene.json");
    // if (save_result != SCENE_SAVE_OK)
    // {
    //     fprintf(stderr, "Failed to save test scene: %d\n", save_result);
    // }
    // =======================================

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

    run_engine_loop(&engine);
    process_timer_log_config_close(&engine.profile_log_config);
    
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
