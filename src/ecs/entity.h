#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t EntityId;

#define INVALID_ENTITY_ID 0

typedef struct EntityRegistry {
    EntityId next_id;
    EntityId *entities;
    size_t count;
    size_t capacity;
} EntityRegistry;

void entity_registry_init(EntityRegistry *registry);
EntityId entity_registry_create(EntityRegistry *registry);
bool entity_registry_destroy(EntityRegistry *registry, EntityId entity);
bool entity_registry_is_alive(const EntityRegistry *registry, EntityId entity);
void entity_registry_shutdown(EntityRegistry *registry);