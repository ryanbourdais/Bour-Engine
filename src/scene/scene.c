#include "scene.h"
#include "entity_factory.h"
#include "scene_serialization.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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
    TransformComponent primitive_transform;
    transform_component_init(&primitive_transform);

    transform_component_set_position(
        &primitive_transform, 
        (vec3s){{3.0f, 1.0f, 0.0f}}
    );

    scene_entity_factory_create_primitive(
        scene, 
        "Primitive Cube", 
        BUILTIN_PRIMITIVE_CUBE, 
        &primitive_transform
    );

    TransformComponent plane_transform;
    transform_component_init(&plane_transform);

    transform_component_set_position(&plane_transform, (vec3s){{0.0f, -0.5f, 0.0f}});

    transform_component_set_scale(&plane_transform, (vec3s){{10.0f, 1.0f, 10.0f}});

    scene_entity_factory_create_primitive(
        scene, 
        "Primitive Plane", 
        BUILTIN_PRIMITIVE_PLANE, 
        &plane_transform
    );

    TransformComponent quad_transform;
    transform_component_init(&quad_transform);

    transform_component_set_position(&quad_transform, (vec3s){{-3.0f, 1.5f, 0.0f}});

    transform_component_set_scale(&quad_transform, (vec3s){{2.0f, 2.0f, 1.0f}});

    scene_entity_factory_create_primitive(
        scene, 
        "Primitive Quad", 
        BUILTIN_PRIMITIVE_QUAD, 
        &quad_transform
    );

    TransformComponent sphere_transform;
    transform_component_init(&sphere_transform);
    transform_component_set_position(&sphere_transform, (vec3s){{0.0f, 1.0f, -3.0f}});

    scene_entity_factory_create_primitive(
        scene, 
        "Primitive UV Sphere", 
        BUILTIN_PRIMITIVE_UV_SPHERE, 
        &sphere_transform
    );

    TransformComponent cylinder_transform;
    transform_component_init(&cylinder_transform);
    transform_component_set_position(&cylinder_transform, (vec3s){{0.0f, 1.0f, 1.0f}});

    scene_entity_factory_create_primitive(
        scene, 
        "Primitive Cylinder", 
        BUILTIN_PRIMITIVE_CYLINDER, 
        &cylinder_transform
    );

    TransformComponent supply_crate_transform;
    transform_component_init(&supply_crate_transform);
    transform_component_set_position(&supply_crate_transform, (vec3s){{3.0f, 1.0f, 1.0f}});

    scene_entity_factory_create_asset(
        scene, 
        "Supply Crate", 
        scene->model_path, 
        &supply_crate_transform
    );

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
    component_storage_init(&scene->skyboxes, sizeof(SkyboxComponent));
}

static bool scene_clone_entities(
    EntityRegistry *out_registry,
    const EntityRegistry *source
)
{
    if (out_registry == NULL || source == NULL)
    {
        return false;
    }

    for (size_t index = 0; index < source->count; index++)
    {
        if (!entity_registry_create_with_id(
                out_registry, 
                source->entities[index]
            ))
        {
            return false;
        }
    }

    out_registry->next_id = source->next_id;
    return true;
}

static bool scene_clone_component_storage(
    ComponentStorage *out_storage,
    const ComponentStorage *source
)
{
    if (out_storage == NULL || source == NULL)
    {
        return false;
    }

    for (size_t index = 0; index < source->count; index++)
    {
        EntityId entity = component_storage_entity_at(source, index);
        const void *component =
            component_storage_at_const(source, index);

        if (component == NULL ||
            !component_storage_add(
                out_storage,
                entity,
                component
            ))
        {
            return false;
        }
    }
    return true;
}

static const char *scene_clone_rebind_owned_path(
    const char *source_path,
    const char source_paths[][SCENE_PATH_MAX_LENGTH],
    char cloned_paths[][SCENE_PATH_MAX_LENGTH],
    size_t path_count
)
{
    if (source_path == NULL)
    {
        return NULL;
    }

    for (size_t index = 0; index < path_count; index++)
    {
        if (source_path == source_paths[index])
        {
            return cloned_paths[index];
        }
    }

    return source_path;
}

static void scene_clone_rebind_asset_paths(
    Scene *clone,
    const Scene *source
)
{
    clone->model_path = scene_clone_rebind_owned_path(
        source->model_path,
        source->loaded_model_paths,
        clone->loaded_model_paths,
        source->loaded_model_path_count
    );

    for (size_t face = 0; face < 6; face++)
    {
        clone->skybox_faces[face] =
            scene_clone_rebind_owned_path(
                source->skybox_faces[face], 
                source->loaded_skybox_faces, 
                clone->loaded_skybox_faces, 
                6
            );
    }

    for (size_t index = 0;
            index < source->mesh_renderers.count;
            index++)
    {
        EntityId entity = component_storage_entity_at(
            &source->mesh_renderers, 
            index
        );

        const MeshRendererComponent *source_mesh =
            component_storage_at_const(
                &source->mesh_renderers, 
                index
            );

        MeshRendererComponent *clone_mesh =
            component_storage_get(
                &clone->mesh_renderers, 
                entity
            );

        if (source_mesh != NULL && clone_mesh != NULL)
        {
            clone_mesh->model_path =
                scene_clone_rebind_owned_path(
                    source_mesh->model_path, 
                    source->loaded_model_paths, 
                    clone->loaded_model_paths, 
                    source->loaded_model_path_count
                );
        }
    }

    for (size_t index = 0;
            index < source->skyboxes.count;
            index++)
    {
        EntityId entity = component_storage_entity_at(
            &source->skyboxes, 
            index
        );

        const SkyboxComponent *source_skybox =
            component_storage_at_const(
                &source->skyboxes, 
                index
            );

        SkyboxComponent *clone_skybox =
            component_storage_get(
                &clone->skyboxes, 
                entity
            );

        if (source_skybox == NULL || clone_skybox == NULL)
        {
            continue;
        }

        for (size_t face = 0; face < 6; face++)
        {
            clone_skybox->faces[face] =
                scene_clone_rebind_owned_path(
                    source_skybox->faces[face], 
                    source->loaded_skybox_faces, 
                    clone->loaded_skybox_faces, 
                    6
                );
        }
    }
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
        RenderableDrawData *renderable =
            &out_config->renderables[out_config->renderable_count];

        switch (mesh_renderer->source_type) 
        {
            case MESH_SOURCE_ASSET:
                if (mesh_renderer->model_path == NULL)
                {
                    continue;
                }

                renderable->geometry_type = RENDERABLE_GEOMETRY_MODEL;
                renderable->model_path = mesh_renderer->model_path;
                renderable->primitive_type = BUILTIN_PRIMITIVE_COUNT;
                renderable->programmable_mesh_id = PROGRAMMABLE_MESH_ID_INVALID;
                renderable->programmable_mesh = NULL;
                break;

            case MESH_SOURCE_PRIMITIVE:
                if (mesh_renderer->primitive_type >= BUILTIN_PRIMITIVE_COUNT)
                {
                    continue;
                }

                renderable->geometry_type = RENDERABLE_GEOMETRY_PRIMITIVE;
                renderable->model_path = NULL;
                renderable->primitive_type = mesh_renderer->primitive_type;
                renderable->programmable_mesh_id = PROGRAMMABLE_MESH_ID_INVALID;
                renderable->programmable_mesh = NULL;
                break;

            case MESH_SOURCE_PROGRAMMABLE:
            {
                ProgrammableMesh *programmable_mesh =
                    programmable_mesh_collection_get(
                        &scene->programmable_meshes, 
                        mesh_renderer->programmable_mesh_id);
                if (mesh_renderer->programmable_mesh_id == PROGRAMMABLE_MESH_ID_INVALID ||
                    programmable_mesh == NULL)
                {
                    continue;
                }

                renderable->geometry_type = RENDERABLE_GEOMETRY_PROGRAMMABLE;
                renderable->model_path = NULL;
                renderable->primitive_type = BUILTIN_PRIMITIVE_COUNT;
                renderable->programmable_mesh_id = mesh_renderer->programmable_mesh_id;
                renderable->programmable_mesh = programmable_mesh;
                break;
            }
            default:
                continue;
        }

        renderable->model_matrix =
            transform_component_model_matrix(transform);

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
    scene->model_path = "assets/models/supply_crate/supply_crate.gltf";
    scene->skybox_faces[0] = "assets/cubemaps/skybox/right.jpg";
    scene->skybox_faces[1] = "assets/cubemaps/skybox/left.jpg";
    scene->skybox_faces[2] = "assets/cubemaps/skybox/top.jpg";
    scene->skybox_faces[3] = "assets/cubemaps/skybox/bottom.jpg";
    scene->skybox_faces[4] = "assets/cubemaps/skybox/front.jpg";
    scene->skybox_faces[5] = "assets/cubemaps/skybox/back.jpg";
}

static void init_scene_camera(Scene *scene)
{
    EntityId camera_entity = entity_registry_create(&scene->entities);

    NameComponent camera_name = {0};
    snprintf(camera_name.value, ENTITY_NAME_MAX_LENGTH, "Editor Camera");

    CameraComponent camera = {
        .fov = 45.0f,
        .near_clip = 0.1f,
        .far_clip = 100.0f
    };

    component_storage_add(&scene->names, camera_entity, &camera_name);
    component_storage_add(&scene->cameras, camera_entity, &camera);

    scene->active_camera = camera_entity;
}

static void init_scene_skybox(Scene *scene)
{
    EntityId skybox_entity = entity_registry_create(&scene->entities);

    NameComponent skybox_name = {0};
    snprintf(skybox_name.value, ENTITY_NAME_MAX_LENGTH, "Editor skybox");

    SkyboxComponent skybox = {
        .faces = {
            scene->skybox_faces[0],
            scene->skybox_faces[1],
            scene->skybox_faces[2],
            scene->skybox_faces[3],
            scene->skybox_faces[4],
            scene->skybox_faces[5],
        }
    };

    component_storage_add(&scene->names, skybox_entity, &skybox_name);
    component_storage_add(&scene->skyboxes, skybox_entity, &skybox);

    scene->active_skybox = skybox_entity;
}

void scene_init_empty(Scene *scene)
{
    init_scene_ecs_storage(scene);

    programmable_mesh_collection_init(&scene->programmable_meshes);

    scene->model_path = NULL;
    scene->loaded_model_path_count = 0;

    for (size_t i = 0; i < MAX_RENDERABLES; i++)
    {
        scene->loaded_model_paths[i][0] = '\0';
    }

    for (int i = 0; i < 6; i++)
    {
        scene->skybox_faces[i] = NULL;
        scene->loaded_skybox_faces[i][0] = '\0';
    }

    scene->active_camera = INVALID_ENTITY_ID;
    scene->active_skybox = INVALID_ENTITY_ID;

    directional_light_init(&scene->legacy_directional_light, (vec3s){{0.0f, -1.0f, 0.0f}}, default_sunlight);

    point_light_collection_init(&scene->legacy_point_lights);
    spot_light_collection_init(&scene->legacy_spot_lights);
    point_light_collection_init(&scene->render_point_lights);
    spot_light_collection_init(&scene->render_spot_lights);
}

bool scene_clone(
    Scene *out_scene,
    const Scene *source
)
{
    if (out_scene == NULL ||
        source == NULL ||
        out_scene == source ||
        source->loaded_model_path_count > MAX_RENDERABLES)
    {
        return false;
    }

    scene_init_empty(out_scene);

    out_scene->active_camera = source->active_camera;
    out_scene->active_skybox = source->active_skybox;

    out_scene->legacy_directional_light =
        source->legacy_directional_light;
    out_scene->legacy_point_lights =
        source->legacy_point_lights;
    out_scene->legacy_spot_lights =
        source->legacy_spot_lights;

    out_scene->model_path = source->model_path;
    out_scene->loaded_model_path_count =
        source->loaded_model_path_count;

    memcpy(
        out_scene->loaded_model_paths,
        source->loaded_model_paths,
        sizeof(out_scene->loaded_model_paths)
    );

    memcpy(
       out_scene->loaded_skybox_faces,
       source->loaded_skybox_faces,
       sizeof(out_scene->loaded_skybox_faces)
    );

    for (size_t face = 0; face < 6; face++)
    {
        out_scene->skybox_faces[face] =
            source->skybox_faces[face];
    }

    if (!scene_clone_entities(
            &out_scene->entities, 
            &source->entities
            ) ||
        !scene_clone_component_storage(
            &out_scene->transforms,
            &source->transforms
            ) ||
        !scene_clone_component_storage(
            &out_scene->names,
            &source->names
            ) ||
        !scene_clone_component_storage(
            &out_scene->mesh_renderers, 
            &source->mesh_renderers
            ) ||
        !programmable_mesh_collection_clone(
            &out_scene->programmable_meshes,
            &source->programmable_meshes
            ) ||
        !scene_clone_component_storage(
            &out_scene->directional_lights,
            &source->directional_lights
            ) ||
        !scene_clone_component_storage(
            &out_scene->point_lights, 
            &source->point_lights
            ) ||
        !scene_clone_component_storage(
            &out_scene->spot_lights, 
            &source->spot_lights
            ) ||
        !scene_clone_component_storage(
            &out_scene->cameras, 
            &source->cameras
            ) ||
        !scene_clone_component_storage(
            &out_scene->skyboxes, 
            &source->skyboxes
            )
       )
       {
           scene_shutdown(out_scene);
           return false;
       }
    scene_clone_rebind_asset_paths(out_scene, source);
    return true;
}

// Initializes built-in baseline scene
void scene_init_default(Scene *scene)
{
    scene_init_empty(scene);

    init_default_scene_assets(scene);
    init_default_scene_lighting(scene);
    init_default_scene_ecs(scene);
    init_scene_camera(scene);
    init_scene_skybox(scene);
}


static void scene_extract_active_skybox(const Scene *scene, SceneRenderConfig *out_config)
{
    const SkyboxComponent *skybox = component_storage_get_const(&scene->skyboxes, scene->active_skybox);

    if (skybox == NULL)
    {
        out_config->skybox_faces[0] = scene->skybox_faces[0];
        out_config->skybox_faces[1] = scene->skybox_faces[1];
        out_config->skybox_faces[2] = scene->skybox_faces[2];
        out_config->skybox_faces[3] = scene->skybox_faces[3];
        out_config->skybox_faces[4] = scene->skybox_faces[4];
        out_config->skybox_faces[5] = scene->skybox_faces[5];
        return;
    }

    out_config->skybox_faces[0] = skybox->faces[0];
    out_config->skybox_faces[1] = skybox->faces[1];
    out_config->skybox_faces[2] = skybox->faces[2];
    out_config->skybox_faces[3] = skybox->faces[3];
    out_config->skybox_faces[4] = skybox->faces[4];
    out_config->skybox_faces[5] = skybox->faces[5];

}

void scene_get_render_config(Scene *scene, SceneRenderConfig *out_config)
{
    scene_extract_renderables(scene, out_config);

    out_config->model_path = scene->model_path;

    for (size_t index = 0; index < scene->mesh_renderers.count; index++)
    {
        const MeshRendererComponent *mesh_renderer =
            component_storage_at_const(
                &scene->mesh_renderers, 
                index
            );

        if (mesh_renderer != NULL &&
            mesh_renderer->source_type == MESH_SOURCE_ASSET &&
            mesh_renderer->model_path != NULL)
        {
            out_config->model_path = mesh_renderer->model_path;
            break;
        }
    }
    
    scene_extract_active_skybox(scene, out_config);

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
}

void scene_shutdown(Scene *scene)
{
    component_storage_shutdown(&scene->cameras);
    component_storage_shutdown(&scene->spot_lights);
    component_storage_shutdown(&scene->point_lights);
    component_storage_shutdown(&scene->directional_lights);
    component_storage_shutdown(&scene->mesh_renderers);
    programmable_mesh_collection_free(&scene->programmable_meshes);
    component_storage_shutdown(&scene->names);
    component_storage_shutdown(&scene->transforms);
    component_storage_shutdown(&scene->skyboxes);

    entity_registry_shutdown(&scene->entities);
}
