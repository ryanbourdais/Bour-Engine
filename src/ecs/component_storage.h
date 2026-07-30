#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "entity.h"
#include "components.h"

typedef struct ComponentStorage {
    EntityId *entities;
    void *components;
    size_t component_size;
    size_t count;
    size_t capacity;
} ComponentStorage;

void component_storage_init(ComponentStorage *storage, size_t component_size);
bool component_storage_add(ComponentStorage *storage, EntityId entity, const void *component);
void *component_storage_get(ComponentStorage *storage, EntityId entity);
bool component_storage_remove(ComponentStorage *storage, EntityId entity);
void component_storage_shutdown(ComponentStorage *storage);
