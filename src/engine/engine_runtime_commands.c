#include "engine_runtime.h"
#include "engine_runtime_internal.h"

#include <stdbool.h>
#include <stdio.h>

#include "../scene/entity_factory.h"
#include "../scene/scene_serialization.h"


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
    const char *name_value
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

    SceneRenderConfig scene_render_config = {0};

    scene_get_render_config(
        &runtime->scene, 
        &scene_render_config
    );

    return scene_entity_factory_create_asset(
        &runtime->scene,
        name,
        scene_render_config.model_path,
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

static EntityId engine_runtime_duplicate_entity(
    EngineRuntime *runtime,
    EntityId entity_id
)
{
    if (!engine_runtime_entity_is_valid(runtime, entity_id))
    {
        return INVALID_ENTITY_ID;
    }

    const TransformComponent *source_transform =
        component_storage_get_const(
            &runtime->scene.transforms, 
            entity_id
        );

    const MeshRendererComponent *source_mesh =
        component_storage_get_const(
            &runtime->scene.mesh_renderers, 
            entity_id
        );

    if (source_transform == NULL || source_mesh == NULL)
    {
        return INVALID_ENTITY_ID;
    }

    TransformComponent duplicate_transform = *source_transform;
    duplicate_transform.position.x += 1.0f;

    if (source_mesh->source_type == MESH_SOURCE_PROGRAMMABLE)
    {
        const ProgrammableMesh *source_programmable_mesh =
            programmable_mesh_collection_get_const(
                &runtime->scene.programmable_meshes, 
                source_mesh->programmable_mesh_id
            );

        if (source_programmable_mesh == NULL ||
            source_programmable_mesh->type !=
                PROGRAMMABLE_MESH_TYPE_PLANE)
        {
            return INVALID_ENTITY_ID;
        }

        return scene_entity_factory_create_programmable_plane(
            &runtime->scene,
            "Duplicated Programmable Plane",
            source_programmable_mesh->plane_width,
            source_programmable_mesh->plane_depth,
            &duplicate_transform
        );
    }
    EntityId duplicate = engine_runtime_create_empty_entity(
        runtime, 
        "Duplicated Entity"
    );

    if (duplicate == INVALID_ENTITY_ID)
    {
        return INVALID_ENTITY_ID;
    }

    TransformComponent *transform =
        component_storage_get(&runtime->scene.transforms, duplicate);

    if (transform == NULL ||
        !component_storage_add(
            &runtime->scene.mesh_renderers, 
            duplicate, 
            source_mesh
        ))
    {
        engine_runtime_delete_entity(runtime, duplicate);
        return INVALID_ENTITY_ID;
    }

    *transform = duplicate_transform;
    return duplicate;
}

static bool engine_runtime_set_light(
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

    vec3s ambient = {{
        command->light_ambient[0],
        command->light_ambient[1],
        command->light_ambient[2],
    }};

    vec3s diffuse = {{
        command->light_diffuse[0],
        command->light_diffuse[1],
        command->light_diffuse[2],
    }};

    vec3s specular = {{
        command->light_specular[0],
        command->light_specular[1],
        command->light_specular[2],
    }};

    vec3s direction = {{
        command->light_direction[0],
        command->light_direction[1],
        command->light_direction[2],
    }};

    vec3s position = {{
        command->light_position[0],
        command->light_position[1],
        command->light_position[2],
    }};

    DirectionalLightComponent *directional =
        component_storage_get(
            &runtime->scene.directional_lights, 
            entity_id
        );

    PointLightComponent *point =
        component_storage_get(
            &runtime->scene.point_lights, 
            entity_id
        );

    SpotLightComponent *spot =
        component_storage_get(
            &runtime->scene.spot_lights, 
            entity_id
        );

    if (directional != NULL)
    {
        directional->light.color.ambient = ambient;
        directional->light.color.diffuse = diffuse;
        directional->light.color.specular = specular;
        directional->light.direction = direction;
        return true;
    }

    if (point != NULL)
    {
        point->light.color.ambient = ambient;
        point->light.color.diffuse = diffuse;
        point->light.color.specular = specular;
        point->light.position = position;
        return true;
    }

    if (spot != NULL)
    {
        spot->light.color.ambient = ambient;
        spot->light.color.diffuse = diffuse;
        spot->light.color.specular = specular;
        spot->light.direction = direction;
        spot->light.position = position;
        return true;
    }

    return false;
}

static const char *engine_runtime_get_scene_path(
    const EngineRuntime *runtime,
    const EngineRuntimeCommand *command
)
{
    if (command != NULL && command->scene_path[0] != '\0')
    {
        return command->scene_path;
    }

    if (runtime != NULL && runtime->has_current_scene_path)
    {
        return runtime->current_scene_path;
    }

    return NULL;
}

static void engine_runtime_set_current_scene_path(
    EngineRuntime *runtime,
    const char *scene_path
)
{
    if (runtime == NULL || scene_path == NULL)
    {
        return;
    }

    if (scene_path == runtime->current_scene_path)
    {
        runtime->has_current_scene_path = true;
        return;
    }

    snprintf(
        runtime->current_scene_path, 
        ENGINE_RUNTIME_SCENE_PATH_MAX_LENGTH, 
        "%s",
        scene_path
    );

    runtime->has_current_scene_path = true;
}

static EngineRuntimeCommandResult engine_runtime_save_scene(
    EngineRuntime *runtime,
    const EngineRuntimeCommand *command
)
{
    const char *scene_path = engine_runtime_get_scene_path(
        runtime, 
        command
    );

    if (scene_path == NULL)
    {
        return ENGINE_RUNTIME_COMMAND_INVALID_ARGUMENT;
    }

    SceneSaveResult save_result =
        scene_save_to_file(&runtime->scene, scene_path);

    if (save_result != SCENE_SAVE_OK)
    {
        fprintf(
            stderr, 
            "Failed to save scene '%s': %d\n",
            scene_path,
            save_result
        );
        
        return ENGINE_RUNTIME_COMMAND_SCENE_IO_FAILED;
    }

    engine_runtime_set_current_scene_path(runtime, scene_path);
    return ENGINE_RUNTIME_COMMAND_OK;
}

static EngineRuntimeCommandResult engine_runtime_load_scene(
    EngineRuntime *runtime,
    const EngineRuntimeCommand *command
)
{
    const char *scene_path = engine_runtime_get_scene_path(
        runtime, 
        command
    );

    if (scene_path == NULL)
    {
        return ENGINE_RUNTIME_COMMAND_INVALID_ARGUMENT;
    }

    SceneLoadResult load_result =
        scene_load_from_file(&runtime->scene, scene_path);

    if (load_result != SCENE_SAVE_OK)
    {
        fprintf(
            stderr, 
            "Failed to load scene '%s': %d\n",
            scene_path,
            load_result
        );

        return ENGINE_RUNTIME_COMMAND_SCENE_IO_FAILED;
    }

    engine_runtime_set_current_scene_path(runtime, scene_path);
    return ENGINE_RUNTIME_COMMAND_OK;
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
        case ENGINE_RUNTIME_COMMAND_DUPLICATE_ENTITY:
        {
            if (!engine_runtime_entity_is_valid(
                    runtime, 
                    command->entity_id
                ))
            {
                return ENGINE_RUNTIME_COMMAND_ENTITY_NOT_FOUND;
            }

            EntityId duplicate = engine_runtime_duplicate_entity(
                runtime, 
                command->entity_id
            );

            if (duplicate == INVALID_ENTITY_ID)
            {
                return ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
            }

            if (out_affected_entity_id != NULL)
            {
                *out_affected_entity_id = duplicate;
            }

            return ENGINE_RUNTIME_COMMAND_OK;
        }
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
        case ENGINE_RUNTIME_COMMAND_SET_LIGHT:
        {
            if (!engine_runtime_entity_is_valid(
                    runtime, 
                    command->entity_id
                ))
            {
                return ENGINE_RUNTIME_COMMAND_ENTITY_NOT_FOUND;
            }

            if (!engine_runtime_set_light(
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
        case ENGINE_RUNTIME_COMMAND_SAVE_SCENE:
            return engine_runtime_save_scene(runtime, command);
        case ENGINE_RUNTIME_COMMAND_LOAD_SCENE:
            return engine_runtime_load_scene(runtime, command);
        default:
            return ENGINE_RUNTIME_COMMAND_OPERATION_FAILED;
    }
}
