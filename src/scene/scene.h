#pragma once

#include <stddef.h>

#include "../renderer/data_types/lightObject.h"
#include "../ecs/entity.h"
#include "../ecs/component_storage.h"
#include "../renderer/renderer_data.h"

typedef struct Scene {
    const char *model_path;
    const char *skybox_faces[6];

    EntityId active_camera;
    EntityId active_skybox;

    EntityRegistry entities;

    DirectionalLight legacy_directional_light;
    PointLightCollection legacy_point_lights;
    SpotLightCollection legacy_spot_lights;

    PointLightCollection render_point_lights;
    SpotLightCollection render_spot_lights;

    ComponentStorage transforms;
    ComponentStorage names;
    ComponentStorage mesh_renderers;
    ComponentStorage directional_lights;
    ComponentStorage point_lights;
    ComponentStorage spot_lights;
    ComponentStorage cameras;
    ComponentStorage skyboxes;
} Scene;

typedef struct SceneRenderConfig {
    const char *model_path;
    const char *skybox_faces[6];

    const DirectionalLight *directional_light;
    const PointLightCollection *point_lights;
    const SpotLightCollection *spot_lights;

    RenderableDrawData renderables[MAX_RENDERABLES];
    size_t renderable_count;
} SceneRenderConfig;

void scene_init_default(Scene *scene);
void scene_init_empty(Scene *scene);
void scene_get_render_config(Scene *scene, SceneRenderConfig *out_config);
void scene_update(Scene *scene, double delta_time);
void scene_shutdown(Scene *scene);
