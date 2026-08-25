#include "entity.h"
#include <stddef.h>
#include <stdlib.h>

void entity_registry_init(EntityRegistry *registry)
{
    registry->capacity = 0;
    registry->count = 0;
    registry->next_id = 1;
    registry->entities = NULL;
}

EntityId entity_registry_create(EntityRegistry *registry)
{
     if (registry->count == registry->capacity) {
            size_t new_capacity = registry->capacity == 0 ? 4 : registry->capacity * 2;
            EntityId *new_entities = realloc(registry->entities, new_capacity * sizeof(EntityId));
            if (new_entities == NULL)
            {
                return INVALID_ENTITY_ID;
            }
            registry->entities = new_entities;
            registry->capacity = new_capacity;
        }
    EntityId next_id = {0};
    next_id = registry->next_id;
    registry->entities[registry->count] = next_id;
    registry->count++;
    registry->next_id++;
    return next_id;
}

bool entity_registry_create_with_id(EntityRegistry *registry, EntityId entity)
{
    if (entity == INVALID_ENTITY_ID || registry == NULL)
    {
        return false;
    }
    if (entity_registry_is_alive(registry, entity))
    {
        return false;
    }

    if (registry->count == registry->capacity) {
        size_t new_capacity = registry->capacity == 0 ? 4 : registry->capacity * 2;
        EntityId *new_entities = realloc(registry->entities, new_capacity * sizeof(EntityId));
        if (new_entities == NULL)
        {
            return false;
        }
        registry->entities = new_entities;
        registry->capacity = new_capacity;
    }
    registry->entities[registry->count] = entity;
    registry->count++;

    if (entity >= registry->next_id)
    {
        registry->next_id = entity + 1;
    }
    
    return true;
}
bool entity_registry_destroy(EntityRegistry *registry, EntityId entity)
{
    for(size_t i = 0; i < registry->count; i++)
    {
        if (registry->entities[i] == entity)
        {
            registry->entities[i] = registry->entities[registry->count - 1];
            registry->count--;
            return true;
        }
    }
    return false;
}

bool entity_registry_is_alive(const EntityRegistry *registry, EntityId entity)
{
    for(size_t i = 0; i < registry->count; i++)
    {
        if (registry->entities[i] == entity)
        {
            return true;
        }
    }
    return false;
}

void entity_registry_shutdown(EntityRegistry *registry)
{
    free(registry->entities);
    registry->capacity = 0;
    registry->count = 0;
    registry->next_id = 1;
    registry->entities = NULL;
}
