#include "engine_runtime.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../renderer/camera.h"
#include "../renderer/renderer.h"
#include "../scene/scene.h"
#include "../scene/scene_serialization.h"
#include "../scene/entity_factory.h"

struct EngineRuntime {
    Camera camera;
    Renderer *renderer;
    Scene scene;
};

static bool engine_runtime_entity_is_valid(
    const EngineRuntime *runtime,
    EntityId entity_id
)
{
    return runtime != NULL &&
        entity_registry_is_alive(
            &runtime->scene.entities, 
            entity_id
        );
}

static EntityId engine_runtime_create_empty_entity(
    EngineRuntime *runtime,
    const char name_value[ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH]
)
{
    EntityId entity = entity_registry_create(&runtime->scene.entities);

    if (entity == INVALID_ENTITY_ID)
    {
        return INVALID_ENTITY_ID;
    }

    NameComponent name = {0};

    if (name_value == NULL || name_value[0] == '\0')
    {
        snprintf(
            name.value, 
            ENTITY_NAME_MAX_LENGTH, 
            "%s",
            "Empty Entity"
        );
    }
    else {
        snprintf(
            name.value, 
            ENTITY_NAME_MAX_LENGTH, 
            "%.*s",
            ENTITY_NAME_MAX_LENGTH - 1,
            name_value
        );
    }

    TransformComponent transform;
    transform_component_init(&transform);

    if (!component_storage_add(
            &runtime->scene.names, 
            entity, 
            &name
        ))
    {
        entity_registry_destroy(&runtime->scene.entities, entity);
        return INVALID_ENTITY_ID;
    }

    if (!component_storage_add(
            &runtime->scene.transforms, 
            entity, 
            &transform
        ))
    {
        component_storage_remove(&runtime->scene.names, entity);
        entity_registry_destroy(&runtime->scene.entities, entity);
        return INVALID_ENTITY_ID;
    }

    return entity;
}

static bool engine_runtime_rename_entity(
    EngineRuntime *runtime,
    EntityId entity_id,
    const char name_value[ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH]
)
{
    if (!engine_runtime_entity_is_valid(runtime, entity_id) ||
            name_value == NULL ||
            name_value[0] == '\0')
    {
        return false;
    }

    NameComponent *name = component_storage_get(
        &runtime->scene.names,
        entity_id
    );

    if (name == NULL)
    {
        NameComponent new_name = {0};

        snprintf(
            new_name.value, 
            ENTITY_NAME_MAX_LENGTH, 
            "%.*s",
            ENTITY_NAME_MAX_LENGTH - 1,
            name_value
        );

        return component_storage_add(
            &runtime->scene.names, 
            entity_id, 
            &new_name
        );
    }

    snprintf(
        name->value, 
        ENTITY_NAME_MAX_LENGTH, 
        "%.*s",
        ENTITY_NAME_MAX_LENGTH - 1,
        name_value
    );

    return true;
}

static bool engine_runtime_set_transform(
    EngineRuntime *runtime,
    EntityId entity_id,
    const EngineRuntimeCommand *command
)
{
    if (!engine_runtime_entity_is_valid(runtime, entity_id) ||
            command == NULL)
    {
        return false;
    }

    TransformComponent *transform = component_storage_get(
        &runtime->scene.transforms, 
        entity_id
    );

    if (transform == NULL)
    {
        return false;
    }

    transform_component_set_position(
        transform, 
        (vec3s){{
            command->position[0],
            command->position[1],
            command->position[2],
        }}
    );

    transform_component_set_rotation(
        transform, 
        (vec3s){{
            command->rotation[0],
            command->rotation[1],
            command->rotation[2]
        }}
    );

    transform_component_set_scale(
        transform, 
        (vec3s){{
            command->scale[0],
            command->scale[1],
            command->scale[2]
        }}
    );

    return true;
}

static EntityId engine_runtime_create_renderable_entity(
    EngineRuntime *runtime,
    const char name_value[ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH]
)
{
    if (runtime == NULL)
    {
        return INVALID_ENTITY_ID;
    }

    char name[ENTITY_NAME_MAX_LENGTH] = {0};

    snprintf(
        name,
        ENTITY_NAME_MAX_LENGTH,
        "%.*s",
        ENTITY_NAME_MAX_LENGTH - 1,
        name_value == NULL || name_value[0] == '\0'
            ? "Renderable Entity"
            : name_value
    );

    TransformComponent transform;
    transform_component_init(&transform);

    return scene_entity_factory_create_asset(
        &runtime->scene,
        name,
        runtime->scene.model_path,
        &transform
    );
}

static EntityId engine_runtime_create_primitive_entity(
    EngineRuntime *runtime,
    EngineRuntimePrimitiveType primitive_type
)
{
    if (runtime == NULL)
    {
        return INVALID_ENTITY_ID;
    }

    BuiltinPrimitiveType builtin_type;
    const char *name = NULL;

    switch (primitive_type)
    {
        case ENGINE_RUNTIME_PRIMITIVE_CUBE:
            builtin_type = BUILTIN_PRIMITIVE_CUBE;
            name = "Cube";
            break;
        
        case ENGINE_RUNTIME_PRIMITIVE_PLANE:
            builtin_type = BUILTIN_PRIMITIVE_PLANE;
            name = "Plane";
            break;
        
        case ENGINE_RUNTIME_PRIMITIVE_QUAD:
            builtin_type = BUILTIN_PRIMITIVE_QUAD;
            name = "Quad";
            break;

        case ENGINE_RUNTIME_PRIMITIVE_UV_SPHERE:
            builtin_type = BUILTIN_PRIMITIVE_UV_SPHERE;
            name = "UV Sphere";
            break;

        case ENGINE_RUNTIME_PRIMITIVE_CYLINDER:
            builtin_type = BUILTIN_PRIMITIVE_CYLINDER;
            name = "Cylinder";
            break;

        default:
            return INVALID_ENTITY_ID;
    }

    TransformComponent transform;
    transform_component_init(&transform);

    return scene_entity_factory_create_primitive(
        &runtime->scene, 
        name, 
        builtin_type, 
        &transform
    );
}

static EntityId engine_runtime_create_programmable_plane(
    EngineRuntime *runtime,
    const EngineRuntimeCommand *command
)
{
    if (runtime == NULL || command == NULL)
    {
        return INVALID_ENTITY_ID;
    }

    float width = command->programmable_plane_width > 0.0f
        ? command->programmable_plane_width
        : 2.0f;

    float depth = command->programmable_plane_depth > 0.0f
        ? command->programmable_plane_depth
        : 2.0f;

    char name[ENTITY_NAME_MAX_LENGTH] = {0};

    snprintf(
        name,
        ENTITY_NAME_MAX_LENGTH,
        "%.*s",
        ENTITY_NAME_MAX_LENGTH - 1,
        command->name[0] == '\0'
            ? "Programmable Plane"
            : command->name
    );

    TransformComponent transform;
    transform_component_init(&transform);

    return scene_entity_factory_create_programmable_plane(
        &runtime->scene, 
        name, 
        width, 
        depth, 
        &transform
    );
}

static bool engine_runtime_delete_entity(
    EngineRuntime *runtime,
    EntityId entity_id
)
{
    if (!engine_runtime_entity_is_valid(runtime, entity_id))
    {
        return false;
    }

    const MeshRendererComponent *mesh_renderer =
        component_storage_get_const(
            &runtime->scene.mesh_renderers, 
            entity_id
        );

    if (mesh_renderer != NULL &&
        mesh_renderer->source_type == MESH_SOURCE_PROGRAMMABLE)
    {
        programmable_mesh_collection_remove(
            &runtime->scene.programmable_meshes, 
            mesh_renderer->programmable_mesh_id
        );
    }

    component_storage_remove(&runtime->scene.names, entity_id);
    component_storage_remove(&runtime->scene.transforms, entity_id);
    component_storage_remove(&runtime->scene.mesh_renderers, entity_id);
    component_storage_remove(&runtime->scene.directional_lights, entity_id);
    component_storage_remove(&runtime->scene.point_lights, entity_id);
    component_storage_remove(&runtime->scene.spot_lights, entity_id);
    component_storage_remove(&runtime->scene.cameras, entity_id);
    component_storage_remove(&runtime->scene.skyboxes, entity_id);

    if (runtime->scene.active_camera == entity_id)
    {
        runtime->scene.active_camera = INVALID_ENTITY_ID;
    }

    if (runtime->scene.active_skybox == entity_id)
    {
        runtime->scene.active_skybox = INVALID_ENTITY_ID;
    }

    return entity_registry_destroy(
        &runtime->scene.entities, 
        entity_id
    );
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

size_t engine_runtime_get_entity_count(
    const EngineRuntime *runtime
)
{
    if (runtime == NULL)
    {
        return 0;
    }

    return runtime->scene.entities.count;
}

size_t engine_runtime_copy_hierarchy(
    const EngineRuntime *runtime,
    EngineRuntimeHierarchyItem *items,
    size_t item_capacity
)
{
    if (runtime == NULL || items == NULL || item_capacity == 0)
    {
        return 0;
    }

    size_t item_count = runtime->scene.entities.count;

    if (item_count > item_capacity)
    {
        item_count = item_capacity;
    }

    for (size_t index = 0; index < item_count; index++)
    {
        EntityId entity = runtime->scene.entities.entities[index];
            
        items[index].entity_id = entity;

        const NameComponent *name = component_storage_get_const(
            &runtime->scene.names, 
            entity
        );

        snprintf(
            items[index].name, 
            ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH, 
            "%s",
            name == NULL ? "Unnamed Entity" : name->value
        );
    }

    return item_count;
}

bool engine_runtime_get_entity_snapshot(
    const EngineRuntime *runtime,
    uint32_t entity_id,
    EngineRuntimeEntitySnapshot *snapshot
)
{
    if (runtime == NULL ||
        snapshot == NULL ||
        !entity_registry_is_alive(&runtime->scene.entities, entity_id))
    {
        return false;
    }

    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->entity_id = entity_id;

    const NameComponent *name = component_storage_get_const(
        &runtime->scene.names, 
        entity_id
    );

    snprintf(
        snapshot->name, 
        ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH, 
        "%s",
        name == NULL ? "Unnamed Entity" : name->value
    );
   
    const TransformComponent *transform = component_storage_get_const(
        &runtime->scene.transforms, 
        entity_id
    );

    if (transform != NULL)
    {
        snapshot->has_transform = true;

        snapshot->position[0] = transform->position.x;
        snapshot->position[1] = transform->position.y;
        snapshot->position[2] = transform->position.z;

        snapshot->rotation[0] = transform->rotation.x;
        snapshot->rotation[1] = transform->rotation.y;
        snapshot->rotation[2] = transform->rotation.z;

        snapshot->scale[0] = transform->scale.x;
        snapshot->scale[1] = transform->scale.y;
        snapshot->scale[2] = transform->scale.z;
    }

    const MeshRendererComponent *mesh_renderer =
        component_storage_get_const(
            &runtime->scene.mesh_renderers, 
            entity_id
        );

    if (mesh_renderer != NULL)
    {
        snapshot->has_mesh_renderer = true;

        if (mesh_renderer->source_type == MESH_SOURCE_PROGRAMMABLE)
        {
            const ProgrammableMesh *programmable_mesh =
                programmable_mesh_collection_get_const(
                    &runtime->scene.programmable_meshes, 
                    mesh_renderer->programmable_mesh_id
                );

            if (programmable_mesh != NULL &&
                programmable_mesh->type ==
                    PROGRAMMABLE_MESH_TYPE_PLANE)
            {
                snapshot->is_programmable_mesh = true;
                snapshot->programmable_mesh_id =
                    mesh_renderer->programmable_mesh_id;
                snapshot->programmable_plane_width =
                    programmable_mesh->plane_width;
                snapshot->programmable_plane_depth =
                    programmable_mesh->plane_depth;
                snapshot->programmable_mesh_dirty =
                    programmable_mesh->dirty;
            }
        }
    }

    snapshot->has_camera = component_storage_get_const(
        &runtime->scene.cameras, 
        entity_id
    ) != NULL;

    snapshot->has_skybox = component_storage_get_const(
        &runtime->scene.skyboxes, 
        entity_id
    ) != NULL;

    const DirectionalLightComponent *directional =
        component_storage_get_const(
            &runtime->scene.directional_lights, 
            entity_id
        );

    const PointLightComponent *point = component_storage_get_const(
        &runtime->scene.point_lights, 
        entity_id
    );

    const SpotLightComponent *spot = component_storage_get_const(
        &runtime->scene.spot_lights, 
        entity_id
    );

    snapshot->has_directional_light = directional != NULL;
    snapshot->has_point_light = point != NULL;
    snapshot->has_spot_light = spot != NULL;

    if (directional != NULL)
    {
        snapshot->light_type = ENGINE_RUNTIME_LIGHT_DIRECTIONAL;

        snapshot->light_ambient[0] = directional->light.color.ambient.x;
        snapshot->light_ambient[1] = directional->light.color.ambient.y;
        snapshot->light_ambient[2] = directional->light.color.ambient.z;

        snapshot->light_diffuse[0] = directional->light.color.diffuse.x;
        snapshot->light_diffuse[1] = directional->light.color.diffuse.y;
        snapshot->light_diffuse[2] = directional->light.color.diffuse.z;

        snapshot->light_specular[0] = directional->light.color.specular.x;
        snapshot->light_specular[1] = directional->light.color.specular.y;
        snapshot->light_specular[2] = directional->light.color.specular.z;

        snapshot->light_direction[0] = directional->light.direction.x;
        snapshot->light_direction[1] = directional->light.direction.y;
        snapshot->light_direction[2] = directional->light.direction.z;
    }
    else if (point != NULL)
    {
        snapshot->light_type = ENGINE_RUNTIME_LIGHT_POINT;

        snapshot->light_ambient[0] = point->light.color.ambient.x;
        snapshot->light_ambient[1] = point->light.color.ambient.y;
        snapshot->light_ambient[2] = point->light.color.ambient.z;

        snapshot->light_diffuse[0] = point->light.color.diffuse.x;
        snapshot->light_diffuse[1] = point->light.color.diffuse.y;
        snapshot->light_diffuse[2] = point->light.color.diffuse.z;

        snapshot->light_specular[0] = point->light.color.specular.x;
        snapshot->light_specular[1] = point->light.color.specular.y;
        snapshot->light_specular[2] = point->light.color.specular.z;

        snapshot->light_position[0] = point->light.position.x;
        snapshot->light_position[1] = point->light.position.y;
        snapshot->light_position[2] = point->light.position.z;
    }
    else if (spot != NULL)
    {
        snapshot->light_type = ENGINE_RUNTIME_LIGHT_SPOT;

        snapshot->light_ambient[0] = spot->light.color.ambient.x;
        snapshot->light_ambient[1] = spot->light.color.ambient.y;
        snapshot->light_ambient[2] = spot->light.color.ambient.z;

        snapshot->light_diffuse[0] = spot->light.color.diffuse.x;
        snapshot->light_diffuse[1] = spot->light.color.diffuse.y;
        snapshot->light_diffuse[2] = spot->light.color.diffuse.z;

        snapshot->light_specular[0] = spot->light.color.specular.x;
        snapshot->light_specular[1] = spot->light.color.specular.y;
        snapshot->light_specular[2] = spot->light.color.specular.z;

        snapshot->light_direction[0] = spot->light.direction.x;
        snapshot->light_direction[1] = spot->light.direction.y;
        snapshot->light_direction[2] = spot->light.direction.z;

        snapshot->light_position[0] = spot->light.position.x;
        snapshot->light_position[1] = spot->light.position.y;
        snapshot->light_position[2] = spot->light.position.z;
    }

    return true;
}

EngineRuntimeCommandResult engine_runtime_execute_command(
    EngineRuntime *runtime, 
    const EngineRuntimeCommand *command, 
    uint32_t *out_affected_entity_id
)
{
    if (out_affected_entity_id != NULL)
    {
        *out_affected_entity_id = INVALID_ENTITY_ID;
    }

    if (runtime == NULL || command == NULL)
    {
        return ENGINE_RUNTIME_COMMAND_INVALID_ARGUMENT;
    }

    switch (command->type)
    {
        case ENGINE_RUNTIME_COMMAND_CREATE_EMPTY_ENTITY:
        {
            EntityId created_entity = engine_runtime_create_empty_entity(
                runtime, 
                command->name
            );

            if (created_entity == INVALID_ENTITY_ID)
            {
                return ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
            }

            if (out_affected_entity_id != NULL)
            {
                *out_affected_entity_id = created_entity;
            }

            return ENGINE_RUNTIME_COMMAND_OK;
        }
        case ENGINE_RUNTIME_COMMAND_CREATE_RENDERABLE_ENTITY:
        {
            EntityId created_entity =
                engine_runtime_create_renderable_entity(
                    runtime, 
                    command->name
                );

            if (created_entity == INVALID_ENTITY_ID)
            {
                return ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
            }

            if (out_affected_entity_id != NULL)
            {
                *out_affected_entity_id = created_entity;
            }

            return ENGINE_RUNTIME_COMMAND_OK;
        }

        case ENGINE_RUNTIME_COMMAND_CREATE_PRIMITIVE_ENTITY:
        {
            EntityId created_entity =
                engine_runtime_create_primitive_entity(
                    runtime, 
                    command->primitive_type
                );

            if (created_entity == INVALID_ENTITY_ID)
            {
                return ENGINE_RUNTIME_COMMAND_INVALID_ARGUMENT;
            }

            if (out_affected_entity_id != NULL)
            {
                *out_affected_entity_id = created_entity;
            }

            return ENGINE_RUNTIME_COMMAND_OK;
        }
        
        case ENGINE_RUNTIME_COMMAND_CREATE_PROGRAMMABLE_PLANE:
        {
            EntityId created_entity =
                engine_runtime_create_programmable_plane(
                    runtime, 
                    command
                );

            if (created_entity == INVALID_ENTITY_ID)
            {
                return ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
            }

            if (out_affected_entity_id != NULL)
            {
                *out_affected_entity_id = created_entity;
            }

            return ENGINE_RUNTIME_COMMAND_OK;
        }

        case ENGINE_RUNTIME_COMMAND_DELETE_ENTITY:
            if (!engine_runtime_entity_is_valid(
                    runtime, 
                    command->entity_id
                ))
            {
                return ENGINE_RUNTIME_COMMAND_ENTITY_NOT_FOUND;
            }

            return engine_runtime_delete_entity(
                runtime, 
                command->entity_id
            )
                ? ENGINE_RUNTIME_COMMAND_OK
                : ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
        case ENGINE_RUNTIME_COMMAND_RENAME_ENTITY:
        {
            if (!engine_runtime_entity_is_valid(
                    runtime,
                    command->entity_id
                ))
            {
                return ENGINE_RUNTIME_COMMAND_ENTITY_NOT_FOUND;
            }

            if (!engine_runtime_rename_entity(
                    runtime,
                    command->entity_id,
                    command->name
                ))
            {
                return ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
            }

            if (out_affected_entity_id != NULL)
            {
                *out_affected_entity_id = command->entity_id;
            }

            return ENGINE_RUNTIME_COMMAND_OK;
        }

        case ENGINE_RUNTIME_COMMAND_SET_TRANSFORM:
        {
            if (!engine_runtime_entity_is_valid(
                    runtime,
                    command->entity_id
                ))
            {
                return ENGINE_RUNTIME_COMMAND_ENTITY_NOT_FOUND;
            }
            
            if (!engine_runtime_set_transform(
                    runtime,
                    command->entity_id,
                    command
                ))
            {
                return ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
            }

            if (out_affected_entity_id != NULL)
            {
                *out_affected_entity_id = command->entity_id;
            }

            return ENGINE_RUNTIME_COMMAND_OK;
        }
        default:
            return ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
    }
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
