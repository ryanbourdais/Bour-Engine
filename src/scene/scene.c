#include "scene.h"

#define ACTIVE_SPOT_LIGHTS 2
#define ACTIVE_POINT_LIGHTS 4

static const LightColor default_sunlight = {
    .ambient  = {{0.02f, 0.02f, 0.02f}},
    .diffuse  = {{0.05f, 0.05f, 0.05f}},
    .specular = {{0.02f, 0.02f, 0.02f}}
};

static const LightColor default_point_light_colors[MAX_SHADER_POINT_LIGHTS] = {
    {
        .ambient  = {{0.00f, 0.00f, 0.00f}},
        .diffuse  = {{3.00f, 0.20f, 0.20f}},
        .specular = {{3.00f, 0.20f, 0.20f}}
    },
    {
        .ambient  = {{0.00f, 0.00f, 0.00f}},
        .diffuse  = {{0.20f, 3.00f, 0.20f}},
        .specular = {{0.20f, 3.00f, 0.20f}}
    },
    {
        .ambient  = {{0.00f, 0.00f, 0.00f}},
        .diffuse  = {{0.20f, 0.20f, 3.00f}},
        .specular = {{0.20f, 0.20f, 3.00f}}
    },
    {
        .ambient  = {{0.00f, 0.00f, 0.00f}},
        .diffuse  = {{3.00f, 1.80f, 0.40f}},
        .specular = {{3.00f, 1.80f, 0.40f}}
    }
};

static const LightColor default_spot_light_colors[MAX_SHADER_SPOT_LIGHTS] = {
    {// Hot magenta/pink
     .ambient = {{0.0f, 0.0f, 0.0f}},
     .diffuse = {{4.0f, 0.0f, 2.5f}},
     .specular = {{4.0f, 0.0f, 2.5f}}},
    {// Electric cyan/blue
     .ambient = {{0.0f, 0.0f, 0.0f}},
     .diffuse = {{0.0f, 3.0f, 4.0f}},
     .specular = {{0.0f, 3.0f, 4.0f}}
    }
};

static const vec3s default_point_light_positions[] = {
    {{ 0.0f,  2.5f,  0.0f}},
    {{ 3.0f,  2.0f,  0.0f}},
    {{-3.0f,  2.0f,  0.0f}},
    {{ 0.0f,  2.0f, -3.0f}}
};

static const vec3s default_spot_light_positions[] = {
    {{ 0.0f, 3.0f,  2.0f}},
    {{ 0.0f, 3.0f, -2.0f}}
};

static const vec3s default_spot_light_directions[] = {
    {{ 0.0f, -1.0f, -0.3f}},
    {{ 0.0f, -1.0f,  0.3f}}
};

static void init_default_scene_lighting(struct Scene *scene)
{
    directional_light_init(&scene->directional_light, (vec3s){{-0.2f, -1.0f, -0.3f}}, default_sunlight);

    point_light_collection_init(&scene->point_lights);

    for (size_t i = 0; i < ACTIVE_POINT_LIGHTS; i++)
    {
        PointLight light = {0};
        point_light_init(&light, default_point_light_positions[i], default_point_light_colors[i], 1.0f, 0.09f, 0.032f);
        point_light_collection_add(&scene->point_lights, light);
    }

    spot_light_collection_init(&scene->spot_lights);

    for (size_t j = 0; j < ACTIVE_SPOT_LIGHTS; j++)
    {
        SpotLight light = {0};
        spot_light_init(&light, default_spot_light_positions[j], default_spot_light_directions[j], default_spot_light_colors[j], 1.0f, 0.09f, 0.032f, 25.0f, 45.0f);
        spot_light_collection_add(&scene->spot_lights, light);
    }
}

void init_default_scene_assets(Scene *scene)
{
    scene->model_path = "assets/models/loft_japanese_11_free_interior/scene.gltf";
    scene->skybox_faces[0] = "assets/cubemaps/skybox/right.jpg";
    scene->skybox_faces[1] = "assets/cubemaps/skybox/left.jpg";
    scene->skybox_faces[2] = "assets/cubemaps/skybox/top.jpg";
    scene->skybox_faces[3] = "assets/cubemaps/skybox/bottom.jpg";
    scene->skybox_faces[4] = "assets/cubemaps/skybox/front.jpg";
    scene->skybox_faces[5] = "assets/cubemaps/skybox/back.jpg";
}

// Initializes built-in baseline scene
void scene_init_default(Scene *scene)
{
    init_default_scene_assets(scene);
    init_default_scene_lighting(scene);
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
    out_config->directional_light = &scene->directional_light;
    out_config->point_lights = &scene->point_lights;
    out_config->spot_lights = &scene->spot_lights;
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
