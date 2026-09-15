#include "entity_factory.h"

#include <stdio.h>

static EntityId scene_entity_factory_create_base(
    Scene *scene,
    const char *name_value,
    const TransformComponent *initial_transform
)
{
    if (scene == NULL ||
        name_value == NULL ||
        initial_transform == NULL )
    {
        return INVALID_ENTITY_ID;
    }

    EntityId entity = entity_registry_create(&scene->entities);

    if (entity == INVALID_ENTITY_ID)
    {
        return INVALID_ENTITY_ID;
    }

    NameComponent name = {0};
    snprintf(name.value, ENTITY_NAME_MAX_LENGTH, "%s", name_value);

    if (!component_storage_add(&scene->names, entity, &name))
    {
        entity_registry_destroy(&scene->entities, entity);
        return INVALID_ENTITY_ID;
    }

    if (!component_storage_add(
            &scene->transforms, 
            entity, 
            initial_transform ))
    {
        component_storage_remove(&scene->names, entity);
        entity_registry_destroy(&scene->entities, entity);
        return INVALID_ENTITY_ID;
    }

    return entity;
}

EntityId scene_entity_factory_create_asset(
    Scene *scene,
    const char *name_value,
    const char *model_path,
    const TransformComponent *initial_transform
)
{
    if (model_path == NULL)
    {
        return  INVALID_ENTITY_ID;
    }

    EntityId entity = scene_entity_factory_create_base(
        scene, 
        name_value, 
        initial_transform
    );

    if (entity == INVALID_ENTITY_ID)
    {
        return INVALID_ENTITY_ID;
    }

    MeshRendererComponent mesh_renderer = {
        .source_type = MESH_SOURCE_ASSET,
        .model_path = model_path,
        .primitive_type = BUILTIN_PRIMITIVE_COUNT,
        .programmable_mesh_id = 0,
    };

    if (!component_storage_add(
            &scene->mesh_renderers, 
            entity,
            &mesh_renderer ))
    {
        return INVALID_ENTITY_ID;
    }

    return entity;
}

EntityId scene_entity_factory_create_primitive(
    Scene *scene,
    const char *name_value,
    BuiltinPrimitiveType primitive_type,
    const TransformComponent *initial_transform
)
{
    if (primitive_type >= BUILTIN_PRIMITIVE_COUNT)
    {
        return INVALID_ENTITY_ID;
    }

    EntityId entity = scene_entity_factory_create_base(
        scene, 
        name_value, 
        initial_transform
    );

    if (entity == INVALID_ENTITY_ID)
    {
        return INVALID_ENTITY_ID;
    }

    MeshRendererComponent mesh_renderer = {
        .source_type = MESH_SOURCE_PRIMITIVE,
        .model_path = NULL,
        .primitive_type = primitive_type,
        .programmable_mesh_id = 0,
    };

    if (!component_storage_add(
            &scene->mesh_renderers, 
            entity, 
            &mesh_renderer ))
    {
        return INVALID_ENTITY_ID;
    }

    return entity;
}


EntityId scene_entity_factory_create_programmable_plane(
    Scene *scene,
    const char *name_value,
    float width,
    float depth,
    const TransformComponent *initial_transform
)
{
    if (scene == NULL ||
        name_value == NULL ||
        initial_transform == NULL)
    {
        return INVALID_ENTITY_ID;
    }

    ProgrammableMeshId mesh_id = PROGRAMMABLE_MESH_ID_INVALID;

    if (!programmable_mesh_collection_create_plane(
            &scene->programmable_meshes, 
            width, 
            depth, 
            &mesh_id))
    {
        return INVALID_ENTITY_ID;
    }

    EntityId entity = scene_entity_factory_create_base(
        scene, 
        name_value, 
        initial_transform
    );

    if (entity == INVALID_ENTITY_ID)
    {
        programmable_mesh_collection_remove(
            &scene->programmable_meshes, 
            mesh_id
        );
        return INVALID_ENTITY_ID;
    }

    MeshRendererComponent mesh_renderer = {
        .source_type = MESH_SOURCE_PROGRAMMABLE,
        .model_path = NULL,
        .primitive_type = BUILTIN_PRIMITIVE_COUNT,
        .programmable_mesh_id = mesh_id,
    };

    if (!component_storage_add(
            &scene->mesh_renderers, 
            entity, 
            &mesh_renderer))
    {
        component_storage_remove(&scene->transforms, entity);
        component_storage_remove(&scene->names, entity);
        entity_registry_destroy(&scene->entities, entity);

        programmable_mesh_collection_remove(
            &scene->programmable_meshes, 
            mesh_id
        );

        return INVALID_ENTITY_ID;
    }

    return entity;
}



