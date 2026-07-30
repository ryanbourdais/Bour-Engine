#include "component_storage.h"

#include <stdlib.h>

void transform_storage_init(TransformStorage *storage)
{
    storage->capacity = 0;
    storage->count = 0;
    storage->components = NULL;
    storage->entities = NULL;
}

static bool transform_storage_grow(TransformStorage *storage)
{
    size_t new_capacity = storage->capacity == 0 ? 4 : storage->capacity * 2;

    EntityId *new_entities = malloc(new_capacity * sizeof(EntityId));
    if (new_entities == NULL)
    {
        return false;
    }

    TransformComponent *new_components = malloc(new_capacity * sizeof(TransformComponent));
    if (new_components == NULL)
    {
        free(new_entities);
        return false;
    }

    for (size_t i = 0; i < storage->count; i++)
    {
        new_entities[i] = storage->entities[i];
        new_components[i] = storage->components[i];
    }

    free(storage->entities);
    free(storage->components);

    storage->entities = new_entities;
    storage->components = new_components;
    storage->capacity = new_capacity;

    return true;
}

bool transform_storage_add(TransformStorage *storage, EntityId entity, TransformComponent component)
{
    if (entity == INVALID_ENTITY_ID)
    {
        return false;
    }

    if (transform_storage_get(storage, entity) != NULL)
    {
        return false;
    }

    if (storage->count == storage->capacity)
    {
        if (!transform_storage_grow(storage))
        {
            return false;
        }
    }

    storage->entities[storage->count] = entity;
    storage->components[storage->count] = component;
    storage->count++;

    return true;
}

TransformComponent *transform_storage_get(TransformStorage *storage, EntityId entity)
{
    for (size_t i = 0; i < storage->count; i++)
    {
        if (storage->entities[i] == entity)
        {
            return &storage->components[i];
        }
    }

    return NULL;
}

bool transform_storage_remove(TransformStorage *storage, EntityId entity)
{
    for (size_t i = 0; i < storage->count; i++)
    {
        if (storage->entities[i] == entity)
        {
            size_t last = storage->count - 1;

            storage->entities[i] = storage->entities[last];
            storage->components[i] = storage->components[last];

            storage->count--;

            return true;
        }
    }

    return false;
}

void transform_storage_shutdown(TransformStorage *storage)
{
    free(storage->entities);
    free(storage->components);

    storage->entities = NULL;
    storage->components = NULL;
    storage->capacity = 0;
    storage->count = 0;
}
