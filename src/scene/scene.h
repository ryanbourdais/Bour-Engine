#pragma once
#include "../renderer/data_types/lightObject.h"
#include "../ecs/entity.h"
#include "../ecs/component_storage.h"
#include "../ecs/components.h"

typedef struct Scene {
    const char *model_path;
    const char *skybox_faces[6];

    EntityRegistry entities;

    DirectionalLight legacy_directional_light;
    PointLightCollection legacy_point_lights;
    SpotLightCollection legacy_spot_lights;

    ComponentStorage transforms;
    ComponentStorage names;
    ComponentStorage mesh_renderers;
    ComponentStorage directional_lights;
    ComponentStorage point_lights;
    ComponentStorage spot_lights;
    ComponentStorage cameras;
} Scene;

typedef struct SceneRenderConfig {
    const char *model_path;
    const char *skybox_faces[6];

    const DirectionalLight *directional_light;
    const PointLightCollection *point_lights;
    const SpotLightCollection *spot_lights;
} SceneRenderConfig;

void scene_init_default(Scene *scene);
void scene_get_render_config(const Scene *scene, SceneRenderConfig *out_config);
void scene_update(Scene *scene, double delta_time);
void scene_shutdown(Scene *scene);
