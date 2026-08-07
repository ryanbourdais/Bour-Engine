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

#define MAX_EDITOR_HIERARCHY_ITEMS 256

typedef struct EngineFrameProfile {
    double engine_update_ms;
    double scene_extract_ms;
    double editor_begin_ms;
    double renderer_ms;
    double editor_render_ms;
    double present_ms;
} EngineFrameProfile;

struct EngineState
{
    GLFWwindow *window;
    // Active runtime/editor camera. ECS CameraComponent is scene data for future editor bridging.
    Camera camera;
    Renderer *renderer;
    Scene scene;

    EngineFrameProfile profile;

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

static double elapsed_ms(double start, double end)
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

        double start = glfwGetTime();
        engine_update(engine);
        engine->profile.engine_update_ms = elapsed_ms(start, glfwGetTime());

        start = glfwGetTime();
        SceneRenderConfig scene_render_config = {0};

        
        scene_get_render_config(&engine->scene, &scene_render_config);
        engine->profile.scene_extract_ms = elapsed_ms(start, glfwGetTime());

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
            
            const TransformComponent *selected_transform = (const TransformComponent *)component_storage_get(&engine->scene.transforms, engine->selected_entity);

            selected_entity_is_renderable = component_storage_get(&engine->scene.mesh_renderers, engine->selected_entity) != NULL;

            if (selected_transform != NULL)
            {
                selected_entity_has_transform = true;

                selected_position[0] = selected_transform->position.x;
                selected_position[1] = selected_transform->position.y;
                selected_position[2] = selected_transform->position.z;
                
                selected_rotation[0] = selected_transform->rotation.x;
                selected_rotation[1] = selected_transform->rotation.y;
                selected_rotation[2] = selected_transform->rotation.z;
            
                selected_scale[0] = selected_transform->scale.x;
                selected_scale[1] = selected_transform->scale.y;
                selected_scale[2] = selected_transform->scale.z;
            }
        }

        DirectionalLightComponent *directional = component_storage_get(&engine->scene.directional_lights, engine->selected_entity);
        PointLightComponent *point = component_storage_get(&engine->scene.point_lights, engine->selected_entity);
        SpotLightComponent *spot = component_storage_get(&engine->scene.spot_lights, engine->selected_entity);

        float selected_light_ambient[3] = {0};
        float selected_light_diffuse[3] = {0};
        float selected_light_specular[3] = {0};
        float selected_light_direction[3] = {0};
        float selected_light_position[3] = {0};

        EditorSelectedLightType selected_light_type = EDITOR_SELECTED_LIGHT_NONE;

        if(directional != NULL)
        {
            selected_light_type = EDITOR_SELECTED_LIGHT_DIRECTIONAL;
            selected_light_ambient[0] = directional->light.color.ambient.r;
            selected_light_ambient[1] = directional->light.color.ambient.g;
            selected_light_ambient[2] = directional->light.color.ambient.b;
            selected_light_diffuse[0] = directional->light.color.diffuse.r;
            selected_light_diffuse[1] = directional->light.color.diffuse.g;
            selected_light_diffuse[2] = directional->light.color.diffuse.b;
            selected_light_specular[0] = directional->light.color.specular.r;
            selected_light_specular[1] = directional->light.color.specular.g;
            selected_light_specular[2] = directional->light.color.specular.b;
            selected_light_direction[0] = directional->light.direction.x;
            selected_light_direction[1] = directional->light.direction.y;
            selected_light_direction[2] = directional->light.direction.z;
        }
        if(point != NULL)
        {
            selected_light_type = EDITOR_SELECTED_LIGHT_POINT;
            selected_light_ambient[0] = point->light.color.ambient.r;
            selected_light_ambient[1] = point->light.color.ambient.g;
            selected_light_ambient[2] = point->light.color.ambient.b;
            selected_light_diffuse[0] = point->light.color.diffuse.r;
            selected_light_diffuse[1] = point->light.color.diffuse.g;
            selected_light_diffuse[2] = point->light.color.diffuse.b;
            selected_light_specular[0] = point->light.color.specular.r;
            selected_light_specular[1] = point->light.color.specular.g;
            selected_light_specular[2] = point->light.color.specular.b;
            selected_light_position[0] = point->light.position.x; 
            selected_light_position[1] = point->light.position.y;
            selected_light_position[2] = point->light.position.z;
        }
        if(spot != NULL)
        {
            selected_light_type = EDITOR_SELECTED_LIGHT_SPOT;
            selected_light_ambient[0] = spot->light.color.ambient.r;
            selected_light_ambient[1] = spot->light.color.ambient.g;
            selected_light_ambient[2] = spot->light.color.ambient.b;
            selected_light_diffuse[0] = spot->light.color.diffuse.r;
            selected_light_diffuse[1] = spot->light.color.diffuse.g;
            selected_light_diffuse[2] = spot->light.color.diffuse.b;
            selected_light_specular[0] = spot->light.color.specular.r;
            selected_light_specular[1] = spot->light.color.specular.g;
            selected_light_specular[2] = spot->light.color.specular.b;
            selected_light_direction[0] = spot->light.direction.x;
            selected_light_direction[1] = spot->light.direction.y;
            selected_light_direction[2] = spot->light.direction.z;
            selected_light_position[0] = spot->light.position.x; 
            selected_light_position[1] = spot->light.position.y;
            selected_light_position[2] = spot->light.position.z;
        }


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
            .editor_cursor_enabled = engine->editor_cursor_enabled,
            .profile_engine_update_ms = engine->profile.engine_update_ms,
            .profile_scene_extract_ms = engine->profile.scene_extract_ms,
            .profile_editor_begin_ms = engine->profile.editor_begin_ms,
            .profile_renderer_ms = engine->profile.renderer_ms,
            .profile_editor_render_ms = engine->profile.editor_render_ms,
            .profile_present_ms = engine->profile.present_ms,
            .hierarchy_items = hierarchy_items,
            .hierarchy_item_count = hierarchy_count,
        };

        RendererFrame frame = {
            .camera = &engine->camera,
            .viewport = {0},
            .renderables = scene_render_config.renderables,
            .renderable_count = scene_render_config.renderable_count,
            .directional_light = scene_render_config.directional_light,
            .spot_lights = scene_render_config.spot_lights,
            .point_lights = scene_render_config.point_lights,
        };

        window_get_framebuffer_size(engine->window, &frame.viewport.width, &frame.viewport.height);

        if (engine->editor_enabled)
        {
            start = glfwGetTime();
            EditorFrameResult editor_result = editor_ui_begin_frame(&editor_frame);

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

            if(result_entity_alive)
            {
                if (editor_result.delete_selected_entity)
                {
                    component_storage_remove(&engine->scene.names, result_entity);
                    component_storage_remove(&engine->scene.transforms, result_entity);
                    component_storage_remove(&engine->scene.mesh_renderers, result_entity);
                    component_storage_remove(&engine->scene.directional_lights, result_entity);
                    component_storage_remove(&engine->scene.spot_lights, result_entity);
                    component_storage_remove(&engine->scene.point_lights, result_entity);
                    component_storage_remove(&engine->scene.cameras, result_entity);
                
                    entity_registry_destroy(&engine->scene.entities, result_entity);

                    engine->selected_entity = INVALID_ENTITY_ID;
                }
                else {
                    if (editor_result.duplicate_selected_entity)
                    {
                        TransformComponent *source_transform = (TransformComponent *)component_storage_get(&engine->scene.transforms, result_entity);
                        
                        MeshRendererComponent *source_mesh = (MeshRendererComponent *)component_storage_get(&engine->scene.mesh_renderers, result_entity);

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

                    if (editor_result.rename_selected_entity)
                    {
                        NameComponent *name = (NameComponent *)component_storage_get(&engine->scene.names, result_entity);

                        if (name == NULL)
                        {
                            NameComponent new_name = {0};
                            snprintf(new_name.value, ENTITY_NAME_MAX_LENGTH, "%s", editor_result.edited_name);
                            component_storage_add(&engine->scene.names, result_entity, &new_name);
                        }
                        else
                        {
                            snprintf(name->value, ENTITY_NAME_MAX_LENGTH, "%s", editor_result.edited_name);
                        }
                    }

                    if (editor_result.transform_changed)
                    {
                        TransformComponent *transform = (TransformComponent *)component_storage_get(&engine->scene.transforms, result_entity);
                        if (transform != NULL)
                        {
                            transform_component_set_position(
                                transform,
                                (vec3s){{editor_result.edited_position[0],
                                        editor_result.edited_position[1],
                                        editor_result.edited_position[2]}}
                            );

                            transform_component_set_rotation(
                                transform,
                                (vec3s){{editor_result.edited_rotation[0],
                                        editor_result.edited_rotation[1],
                                        editor_result.edited_rotation[2]}}
                            );

                            transform_component_set_scale(
                                transform,
                                (vec3s){{editor_result.edited_scale[0],
                                        editor_result.edited_scale[1],
                                        editor_result.edited_scale[2]}}
                            );
                        }
                    }
                
                    if (editor_result.light_changed)
                    {
                        DirectionalLightComponent *directional = component_storage_get(&engine->scene.directional_lights, result_entity);
                        PointLightComponent *point = component_storage_get(&engine->scene.point_lights, result_entity);
                        SpotLightComponent *spot = component_storage_get(&engine->scene.spot_lights, result_entity);
                    
                        vec3s ambient = {{
                            editor_result.edited_light_ambient[0],
                            editor_result.edited_light_ambient[1],
                            editor_result.edited_light_ambient[2]
                        }};
                        vec3s diffuse = {{
                            editor_result.edited_light_diffuse[0],
                            editor_result.edited_light_diffuse[1],
                            editor_result.edited_light_diffuse[2]
                        }};
                        vec3s specular = {{
                            editor_result.edited_light_specular[0],
                            editor_result.edited_light_specular[1],
                            editor_result.edited_light_specular[2]
                        }};

                        vec3s direction = {0};
                        vec3s position = {0};
                        if (directional)
                        {
                            direction = (vec3s){{
                                editor_result.edited_light_direction[0],
                                editor_result.edited_light_direction[1],
                                editor_result.edited_light_direction[2],
                            }};
                        }
                        else {
                            position = (vec3s){{
                                editor_result.edited_light_position[0],
                                editor_result.edited_light_position[1],
                                editor_result.edited_light_position[2],
                            }};
                        }

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
                            spot->light.position = position;
                        }
                    }
                }
            }

            engine->profile.editor_begin_ms = elapsed_ms(start, glfwGetTime());

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
        start = glfwGetTime();
        renderer_render_frame(engine->renderer, &frame);
        engine->profile.renderer_ms = elapsed_ms(start, glfwGetTime());
        if (engine->editor_enabled)
        {
            start = glfwGetTime();
            editor_ui_render();
            engine->profile.editor_render_ms = elapsed_ms(start, glfwGetTime());
        }
        start = glfwGetTime();
        window_present(engine->window);
        engine->profile.present_ms = elapsed_ms(start, glfwGetTime());
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
