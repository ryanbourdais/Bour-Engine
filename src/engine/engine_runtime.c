#include "engine_runtime.h"
#include "engine_runtime_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../scene/scene_serialization.h"
#include "../core/project_paths.h"

static bool engine_runtime_initialize_renderer(
    EngineRuntime *runtime,
    int framebuffer_width,
    int framebuffer_height
)
{
    runtime->renderer = renderer_create();

    if (runtime->renderer == NULL)
    {
        return false;
    }

    SceneRenderConfig scene_render_config = {0};

    scene_get_render_config(
        &runtime->scene, 
        &scene_render_config
    );

    RendererConfig renderer_config = {
        .viewport = {
            .width = framebuffer_width,
            .height = framebuffer_height,
        },
        .project_root = runtime->project_root,
        .shader_directory = runtime->renderer_shader_directory,
        .camera = &runtime->camera,
        .entity_id_picking_enabled = runtime->entity_id_picking_enabled,
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

    if (renderer_init(runtime->renderer, &renderer_config) != 0)
    {
        renderer_destroy(runtime->renderer);
        runtime->renderer = NULL;
        return false;
    }

    return true;
}

EngineRuntime *engine_runtime_create(
    const EngineRuntimeCreateInfo *create_info
)
{
    if (create_info == NULL ||
        create_info->framebuffer_width <= 0 ||
        create_info->framebuffer_height <= 0)
    {
        return NULL;
    }
    
    EngineRuntime *runtime = calloc(1, sizeof(EngineRuntime));

    if (runtime == NULL)
    {
        return NULL;
    }

    const char *project_root = create_info->project_root;

    if (project_root == NULL || project_root[0] == '\0')
    {
        project_root = ".";
    }

    int project_root_length = snprintf(
        runtime->project_root,
        sizeof(runtime->project_root),
        "%s",
        project_root
    );

    if (project_root_length < 0 ||
        (size_t)project_root_length >= sizeof(runtime->project_root))
    {
        fprintf(stderr, "Project root path is too long\n");
        free(runtime);
        return NULL;
    }

    const char *renderer_shader_directory = create_info->renderer_shader_directory;

    if (renderer_shader_directory == NULL ||
        renderer_shader_directory[0] == '\0')
    {
        renderer_shader_directory = "src/renderer/shaders";
    }

    int renderer_shader_directory_length = snprintf(
        runtime->renderer_shader_directory, 
        sizeof(runtime->renderer_shader_directory), 
        "%s",
        renderer_shader_directory
    );

    if (renderer_shader_directory_length < 0 ||
        (size_t)renderer_shader_directory_length >=
            sizeof(runtime->renderer_shader_directory))
    {
        fprintf(stderr, "Renderer shader directory is too long\n");
        free(runtime);
        return NULL;
    }

    camera_init(&runtime->camera);
    camera_update(&runtime->camera);

    scene_init_default(&runtime->scene);

    runtime->entity_id_picking_enabled = create_info->entity_id_picking_enabled;

    if (create_info->scene_path != NULL)
    {
        if (strlen(create_info->scene_path) >=
            sizeof(runtime->current_scene_path))
        {
            fprintf(stderr, "Scene path is too long: %s\n", create_info->scene_path);
            scene_shutdown(&runtime->scene);
            free(runtime);
            return NULL;
        }

        char resolved_scene_path[
            ENGINE_RUNTIME_PROJECT_ROOT_MAX_LENGTH +
            ENGINE_RUNTIME_SCENE_PATH_MAX_LENGTH
        ];

        if (project_path_resolve(
                runtime->project_root, 
                create_info->scene_path, 
                resolved_scene_path, 
                sizeof(resolved_scene_path)
            ) != PROJECT_PATH_OK)
        {
            fprintf(stderr, "Invalid scene path: %s\n", create_info->scene_path);
            scene_shutdown(&runtime->scene);
            free(runtime);
            return NULL;
        }

        SceneLoadResult scene_load_result =
            scene_load_from_file(&runtime->scene, resolved_scene_path);

        if (scene_load_result != SCENE_LOAD_OK)
        {
            fprintf(
                stderr, 
                "Failed to load scene '%s' from project root '%s': %d\n",
                create_info->scene_path,
                runtime->project_root,
                scene_load_result
            );
            scene_shutdown(&runtime->scene);
            free(runtime);
            return NULL;
        }
    }

    if (create_info->scene_path != NULL)
    {
        snprintf(
            runtime->current_scene_path, 
            ENGINE_RUNTIME_SCENE_PATH_MAX_LENGTH, 
            "%s",
            create_info->scene_path
        );

        runtime->has_current_scene_path = true;
    }

    if (!engine_runtime_initialize_renderer(
            runtime, 
            create_info->framebuffer_width, 
            create_info->framebuffer_height
        ))
    {
        scene_shutdown(&runtime->scene);
        free(runtime);
        return NULL;
    }

    return runtime;
}

EngineRuntime *engine_runtime_create_preview(
    const EngineRuntime *authoring_runtime,
    int framebuffer_width,
    int framebuffer_height
)
{
    if (authoring_runtime == NULL ||
        framebuffer_width <= 0 ||
        framebuffer_height <= 0)
    {
        return NULL;
    }

    EngineRuntime *preview = calloc(1, sizeof(EngineRuntime));

    if (preview == NULL)
    {
        return NULL;
    }

   snprintf(
        preview->project_root, 
        sizeof(preview->project_root), 
        "%s",
        authoring_runtime->project_root
    ); 

   snprintf(
        preview->renderer_shader_directory, 
        sizeof(preview->renderer_shader_directory), 
        "%s",
        authoring_runtime->renderer_shader_directory
    );

    preview->camera = authoring_runtime->camera;
    camera_update(&preview->camera);

    if (!scene_clone(
            &preview->scene, 
            &authoring_runtime->scene
        ))
    {
        free(preview);
        return NULL;
    }

    if (authoring_runtime->has_current_scene_path)
    {
        snprintf(
            preview->current_scene_path, 
            ENGINE_RUNTIME_SCENE_PATH_MAX_LENGTH, 
            "%s",
            authoring_runtime->current_scene_path
        );

        preview->has_current_scene_path = true;    
    }

    preview->entity_id_picking_enabled = authoring_runtime->entity_id_picking_enabled;

    if (!engine_runtime_initialize_renderer(
            preview, 
            framebuffer_width, 
            framebuffer_height
        ))
    {
        scene_shutdown(&preview->scene);
        free(preview);
        return NULL;
    }

    return preview;
}

void engine_runtime_destroy(EngineRuntime *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    renderer_shutdown(runtime->renderer);
    renderer_destroy(runtime->renderer);
    scene_shutdown(&runtime->scene);
    free(runtime);
}


void engine_runtime_update(
    EngineRuntime *runtime,
    double delta_time,
    const EngineRuntimeInput *input
)
{
    if (runtime == NULL)
    {
        return;
    }

    if (input == NULL || input->simulation_update_enabled)
    {
        scene_update(&runtime->scene, delta_time);
    }

    if (input != NULL && input->camera_input_enabled)
    {
        vec2s movement_axis = {{
            input->movement_x,
            input->movement_y,
        }};

        vec2s mouse_offsets = {{
            input->mouse_delta_x,
            input->mouse_delta_y,
        }};

        camera_movement(
            &runtime->camera, 
            movement_axis,
            (float)delta_time
        );
        handle_mouse(&runtime->camera, mouse_offsets, true);
    }

    camera_update(&runtime->camera);
}

void engine_runtime_render(
    EngineRuntime *runtime,
    const EngineRuntimeRenderTarget *target
)
{
    if (runtime == NULL ||
        target == NULL ||
        target->framebuffer_width <= 0 ||
        target->framebuffer_height <= 0)
    {
        return;
    }

    SceneRenderConfig scene_render_config = {0};
    scene_get_render_config(&runtime->scene, &scene_render_config);

    RendererFrame frame = {
        .camera = &runtime->camera,
        .viewport = {
            .width = target->framebuffer_width,
            .height = target->framebuffer_height,
        },
        .present_to_default_framebuffer =
            target->type ==
            ENGINE_RUNTIME_RENDER_TARGET_DEFAULT_FRAMEBUFFER,
        .renderables = scene_render_config.renderables,
        .renderable_count = scene_render_config.renderable_count,
        .directional_light = scene_render_config.directional_light,
        .point_lights = scene_render_config.point_lights,
        .spot_lights = scene_render_config.spot_lights,
    };

    renderer_render_frame(runtime->renderer, &frame);
}

uint32_t engine_runtime_get_resolved_texture(
    const EngineRuntime *runtime
)
{
    if (runtime == NULL || runtime->renderer == NULL)
    {
        return 0;
    }

    return renderer_get_resolved_scene_texture(runtime->renderer);
}

uint32_t engine_runtime_pick_entity(
    const EngineRuntime *runtime,
    float viewport_u,
    float viewport_v
)
{
    if (runtime == NULL)
    {
        return ENGINE_RUNTIME_INVALID_ENTITY_ID;
    }

    return renderer_pick_entity(
        runtime->renderer, 
        viewport_u, 
        viewport_v
    );
}

EngineRuntimeRenderStats engine_runtime_get_render_stats(
    EngineRuntime *runtime
)
{
    EngineRuntimeRenderStats runtime_stats = {0};

    if (runtime == NULL || runtime->renderer == NULL)
    {
        return runtime_stats;
    }

    SceneRenderConfig scene_render_config = {0};
    scene_get_render_config(&runtime->scene, &scene_render_config);

    RendererFrame frame = {
        .camera = &runtime->camera,
        .viewport = {0},
        .present_to_default_framebuffer = false,
        .renderables = scene_render_config.renderables,
        .renderable_count = scene_render_config.renderable_count,
        .directional_light = scene_render_config.directional_light,
        .point_lights = scene_render_config.point_lights,
        .spot_lights = scene_render_config.spot_lights,
    };

    RendererStats renderer_stats = renderer_get_frame_stats(
        runtime->renderer, 
        &frame
    );

    runtime_stats.renderable_count  = scene_render_config.renderable_count;
    runtime_stats.mesh_count = renderer_stats.mesh_count;
    runtime_stats.vertex_count = renderer_stats.vertex_count;
    runtime_stats.triangle_count = renderer_stats.triangle_count;
    runtime_stats.texture_count = renderer_stats.texture_count;
    runtime_stats.submitted_draw_count =
        renderer_stats.submitted_draw_count;
    runtime_stats.submitted_mesh_count =
        renderer_stats.submitted_mesh_count;
    runtime_stats.submitted_vertex_count =
        renderer_stats.submitted_vertex_count;
    runtime_stats.submitted_triangle_count =
      renderer_stats.submitted_triangle_count;
    runtime_stats.missing_model_count =
      renderer_stats.missing_model_count;
    runtime_stats.viewport_width = renderer_stats.viewport_width;
    runtime_stats.viewport_height = renderer_stats.viewport_height;
    runtime_stats.render_target_resize_count =
        renderer_stats.render_target_resize_count;
    runtime_stats.render_target_noop_count =
        renderer_stats.render_target_noop_count;
    runtime_stats.zero_size_viewport_count =
        renderer_stats.zero_size_viewport_count;

    return runtime_stats;
}
