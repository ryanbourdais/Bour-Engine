#include "scene.h"

#include <stdio.h>

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

static void init_default_scene_ecs(Scene *scene)
{
    EntityId model_entity = entity_registry_create(&scene->entities);

    NameComponent name = {0};
    snprintf(name.value, ENTITY_NAME_MAX_LENGTH, "Loft Model");

    TransformComponent transform;
    transform_component_init(&transform);
    MeshRendererComponent mesh_renderer = {
        .model_path = "assets/models/loft_japanese_11_free_interior/scene.gltf"
    };

    component_storage_add(&scene->names, model_entity, &name);
    component_storage_add(&scene->transforms, model_entity, &transform);
    component_storage_add(&scene->mesh_renderers, model_entity, &mesh_renderer);

    EntityId sun_entity = entity_registry_create(&scene->entities);

    NameComponent sun_name = {0};
    snprintf(sun_name.value, ENTITY_NAME_MAX_LENGTH, "Sun");

    DirectionalLightComponent sun = {
        .light = scene->legacy_directional_light
    };

    component_storage_add(&scene->names, sun_entity, &sun_name);
    component_storage_add(&scene->directional_lights, sun_entity, &sun);

    for (size_t i = 0; i < scene->legacy_point_lights.count; i++)
    {
        EntityId entity = entity_registry_create(&scene->entities);

        NameComponent name = {0};
        snprintf(name.value, ENTITY_NAME_MAX_LENGTH, "Point Light %zu", i);

        PointLightComponent light = {
            .light = scene->legacy_point_lights.items[i]
        };

        component_storage_add(&scene->names, entity, &name);
        component_storage_add(&scene->point_lights, entity, &light);
    }

    for (size_t i = 0; i < scene->legacy_spot_lights.count; i++)
    {
        EntityId entity = entity_registry_create(&scene->entities);

        NameComponent name = {0};
        snprintf(name.value, ENTITY_NAME_MAX_LENGTH, "Spot Light %zu", i);

        SpotLightComponent light = {
            .light = scene->legacy_spot_lights.items[i]
        };

        component_storage_add(&scene->names, entity, &name);
        component_storage_add(&scene->spot_lights, entity, &light);
    }

}

static void init_scene_ecs_storage(Scene *scene)
{
    entity_registry_init(&scene->entities);

    component_storage_init(&scene->transforms, sizeof(TransformComponent));
    component_storage_init(&scene->names, sizeof(NameComponent));
    component_storage_init(&scene->mesh_renderers, sizeof(MeshRendererComponent));
    component_storage_init(&scene->directional_lights, sizeof(DirectionalLightComponent));
    component_storage_init(&scene->point_lights, sizeof(PointLightComponent));
    component_storage_init(&scene->spot_lights, sizeof(SpotLightComponent));
    component_storage_init(&scene->cameras, sizeof(CameraComponent));
}

static void scene_extract_point_lights(Scene *scene)
{
    point_light_collection_init(&scene->render_point_lights);

    for (size_t i = 0; i < scene->point_lights.count; i++)
    {
        const PointLightComponent *component = (const PointLightComponent *)component_storage_at_const(&scene->point_lights, i);

        if (component == NULL)
        {
            continue;
        }

        point_light_collection_add(&scene->render_point_lights, component->light);
    }
}

static void scene_extract_spot_lights(Scene *scene)
{
    spot_light_collection_init(&scene->render_spot_lights);

    for (size_t i = 0; i < scene->spot_lights.count; i++)
    {
        const SpotLightComponent *component = (const SpotLightComponent *)component_storage_at_const(&scene->spot_lights, i);

        if (component == NULL)
        {
            continue;
        }

        spot_light_collection_add(&scene->render_spot_lights, component->light);
    }
}

static void scene_extract_renderables(Scene *scene, SceneRenderConfig *out_config)
{
    out_config->renderable_count = 0;

    for (size_t i = 0; i < scene->mesh_renderers.count; i++)
    {
        if (out_config->renderable_count >= MAX_RENDERABLES)
        {
            break;
        }

        EntityId entity = component_storage_entity_at(&scene->mesh_renderers, i);

        const MeshRendererComponent *mesh_renderer = component_storage_at_const(&scene->mesh_renderers, i);
        
        const TransformComponent *transform = component_storage_get(&scene->transforms, entity);

        if (mesh_renderer == NULL || transform == NULL)
        {
            continue;
        }

        RenderableDrawData *renderable = &out_config->renderables[out_config->renderable_count];
        
        renderable->model_path = mesh_renderer->model_path;
        renderable->model_matrix = transform_component_model_matrix(transform);

        out_config->renderable_count++;
    }
}

static void init_default_scene_lighting(struct Scene *scene)
{
    directional_light_init(&scene->legacy_directional_light, (vec3s){{-0.2f, -1.0f, -0.3f}}, default_sunlight);

    point_light_collection_init(&scene->legacy_point_lights);

    for (size_t i = 0; i < ACTIVE_POINT_LIGHTS; i++)
    {
        PointLight light = {0};
        point_light_init(&light, default_point_light_positions[i], default_point_light_colors[i], 1.0f, 0.09f, 0.032f);
        point_light_collection_add(&scene->legacy_point_lights, light);
    }

    spot_light_collection_init(&scene->legacy_spot_lights);

    for (size_t j = 0; j < ACTIVE_SPOT_LIGHTS; j++)
    {
        SpotLight light = {0};
        spot_light_init(&light, default_spot_light_positions[j], default_spot_light_directions[j], default_spot_light_colors[j], 1.0f, 0.09f, 0.032f, 25.0f, 45.0f);
        spot_light_collection_add(&scene->legacy_spot_lights, light);
    }
}

static void init_default_scene_assets(Scene *scene)
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
    init_scene_ecs_storage(scene);

    init_default_scene_assets(scene);
    init_default_scene_lighting(scene);
    init_default_scene_ecs(scene);
}

void scene_get_render_config(Scene *scene, SceneRenderConfig *out_config)
{
    scene_extract_renderables(scene, out_config);

    const MeshRendererComponent *mesh_renderer = (const MeshRendererComponent *)component_storage_first_const(&scene->mesh_renderers);

    out_config->model_path = mesh_renderer != NULL ? mesh_renderer->model_path : scene->model_path;

    out_config->skybox_faces[0] = scene->skybox_faces[0];
    out_config->skybox_faces[1] = scene->skybox_faces[1];
    out_config->skybox_faces[2] = scene->skybox_faces[2];
    out_config->skybox_faces[3] = scene->skybox_faces[3];
    out_config->skybox_faces[4] = scene->skybox_faces[4];
    out_config->skybox_faces[5] = scene->skybox_faces[5];

    const DirectionalLightComponent *sun = (const DirectionalLightComponent *)component_storage_first_const(&scene->directional_lights);

    out_config->directional_light = sun != NULL ? &sun->light : &scene->legacy_directional_light;

    scene_extract_point_lights(scene);
    scene_extract_spot_lights(scene);

    out_config->point_lights = &scene->render_point_lights;
    out_config->spot_lights = &scene->render_spot_lights;
}

void scene_update(Scene *scene, double delta_time)
{
    (void)scene;
    (void)delta_time;
    EntityId entity = component_storage_entity_at(&scene->mesh_renderers,0);
    TransformComponent *transform = component_storage_get(&scene->transforms, entity);
    if (transform != NULL)
    {
        transform->rotation.y += (float)delta_time * 15.0f;
    }
}

void scene_shutdown(Scene *scene)
{
    component_storage_shutdown(&scene->cameras);
    component_storage_shutdown(&scene->spot_lights);
    component_storage_shutdown(&scene->point_lights);
    component_storage_shutdown(&scene->directional_lights);
    component_storage_shutdown(&scene->mesh_renderers);
    component_storage_shutdown(&scene->names);
    component_storage_shutdown(&scene->transforms);

    entity_registry_shutdown(&scene->entities);
}
