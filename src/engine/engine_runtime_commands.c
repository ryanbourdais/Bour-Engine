#include "engine_runtime_internal.h"

#include <stdio.h>

#include "../scene/entity_factory.h"


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
