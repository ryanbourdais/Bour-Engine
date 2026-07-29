#pragma once
#include "../renderer/data_types/lightObject.h"

typedef struct Scene {
    const char *model_path;
    const char *skybox_faces[6];

    DirectionalLight directional_light;
    PointLightCollection point_lights;
    SpotLightCollection spot_lights;
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
