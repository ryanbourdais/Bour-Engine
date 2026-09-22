#include "engine_runtime.h"

#include <stdlib.h>

#include "../renderer/camera.h"
#include "../renderer/renderer.h"
#include "../scene/scene.h"
#include "../scene/scene_serialization.h"

struct EngineRuntime {
    Camera camera;
    Renderer *renderer;
    Scene scene;
};

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

    camera_init(&runtime->camera);
    camera_update(&runtime->camera);

    scene_init_default(&runtime->scene);

    if (create_info->scene_path != NULL &&
            scene_load_from_file(&runtime->scene, create_info->scene_path) !=
                SCENE_LOAD_OK)
    {
        scene_shutdown(&runtime->scene);
        free(runtime);
        return NULL;
    }

    runtime->renderer = renderer_create();

    if (runtime->renderer == NULL)
    {
        scene_shutdown(&runtime->scene);
        free(runtime);
        return NULL;
    }

    SceneRenderConfig scene_render_config = {0};
    scene_get_render_config(&runtime->scene, &scene_render_config);

    RendererConfig renderer_config = {
        .viewport = {
            .width = create_info->framebuffer_width,
            .height = create_info->framebuffer_height,
        },
        .camera = &runtime->camera,
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

    if (renderer_init(runtime->renderer, &renderer_config) != 0)
    {
        renderer_destroy(runtime->renderer);
        scene_shutdown(&runtime->scene);
        free(runtime);
        return NULL;
    }

    return runtime;
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

    scene_update(&runtime->scene, delta_time);

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
