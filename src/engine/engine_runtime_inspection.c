#include "engine_runtime_internal.h"

#include <stdio.h>
#include <string.h>


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

