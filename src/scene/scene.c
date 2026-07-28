#include "scene.h"

void scene_init_default(Scene *scene)
{
    scene->model_path = "assets/models/loft_japanese_11_free_interior/scene.gltf";
    scene->skybox_faces[0] = "assets/cubemaps/skybox/right.jpg";
    scene->skybox_faces[1] = "assets/cubemaps/skybox/left.jpg";
    scene->skybox_faces[2] = "assets/cubemaps/skybox/top.jpg";
    scene->skybox_faces[3] = "assets/cubemaps/skybox/bottom.jpg";
    scene->skybox_faces[4] = "assets/cubemaps/skybox/front.jpg";
    scene->skybox_faces[5] = "assets/cubemaps/skybox/back.jpg";
}

void scene_get_render_config(const Scene *scene, SceneRenderConfig *out_config)
{
    out_config->model_path = scene->model_path;
    out_config->skybox_faces[0] = scene->skybox_faces[0];
    out_config->skybox_faces[1] = scene->skybox_faces[1];
    out_config->skybox_faces[2] = scene->skybox_faces[2];
    out_config->skybox_faces[3] = scene->skybox_faces[3];
    out_config->skybox_faces[4] = scene->skybox_faces[4];
    out_config->skybox_faces[5] = scene->skybox_faces[5];
}

void scene_update(Scene *scene, double delta_time)
{
    (void)scene;
    (void)delta_time;
}

void scene_shutdown(Scene *scene)
{
    (void)scene;
}
