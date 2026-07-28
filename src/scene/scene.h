#pragma once

typedef struct Scene {
    const char *model_path;
    const char *skybox_faces[6];
} Scene;

typedef struct SceneRenderConfig {
    const char *model_path;
    const char *skybox_faces[6];
} SceneRenderConfig;

void scene_init_default(Scene *scene);
void scene_get_render_config(const Scene *scene, SceneRenderConfig *out_config);
void scene_update(Scene *scene, double delta_time);
void scene_shutdown(Scene *scene);
