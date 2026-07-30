#include "component_storage.h"

#include <stdlib.h>
#include <string.h>

void component_storage_init(ComponentStorage *storage, size_t component_size)
{
    storage->capacity = 0;
    storage->count = 0;
    storage->components = NULL;
    storage->entities = NULL;
    storage->component_size = component_size;
}

static bool component_storage_grow(ComponentStorage *storage)
{
    size_t new_capacity = storage->capacity == 0 ? 4 : storage->capacity * 2;

    EntityId *new_entities = malloc(new_capacity * sizeof(EntityId));
    if (new_entities == NULL)
    {
        return false;
    }

    void *new_components = malloc(new_capacity * storage->component_size);
    if (new_components == NULL)
    {
        free(new_entities);
        return false;
    }

    for (size_t i = 0; i < storage->count; i++)
    {
        new_entities[i] = storage->entities[i];
    }

    if (storage->components != NULL)
    {
        memcpy(new_components, storage->components, storage->count * storage->component_size);
    }

    free(storage->entities);
    free(storage->components);

    storage->entities = new_entities;
    storage->components = new_components;
    storage->capacity = new_capacity;

    return true;
}

bool component_storage_add(ComponentStorage *storage, EntityId entity, const void *component)
{
    if (entity == INVALID_ENTITY_ID)
    {
        return false;
    }

    if (component == NULL)
    {
        return false;
    }

    if (storage->component_size == 0)
    {
        return false;
    }

    if (component_storage_get(storage, entity) != NULL)
    {
        return false;
    }

    if (storage->count == storage->capacity)
    {
        if (!component_storage_grow(storage))
        {
            return false;
        }
    }

    storage->entities[storage->count] = entity;

    char *components = storage->components;
    void *destination = components + storage->count * storage->component_size;

    memcpy(destination, component, storage->component_size);

    storage->count++;

    return true;
}

void *component_storage_get(ComponentStorage *storage, EntityId entity)
{
    char *components = storage->components;

    for (size_t i = 0; i < storage->count; i++)
    {
        if (storage->entities[i] == entity)
        {
            return components + i * storage->component_size;
        }
    }

    return NULL;
}

bool component_storage_remove(ComponentStorage *storage, EntityId entity)
{
    char *components = storage->components;

    for (size_t i = 0; i < storage->count; i++)
    {
        if (storage->entities[i] == entity)
        {
            size_t last = storage->count - 1;

            storage->entities[i] = storage->entities[last];

            void *destination = components + i * storage->component_size;
            void *source = components + last * storage->component_size;

            memcpy(destination, source, storage->component_size);

            storage->count--;

            return true;
        }
    }

    return false;
}

void component_storage_shutdown(ComponentStorage *storage)
{
    free(storage->entities);
    free(storage->components);

    storage->entities = NULL;
    storage->components = NULL;
    storage->component_size = 0;
    storage->capacity = 0;
    storage->count = 0;
}
