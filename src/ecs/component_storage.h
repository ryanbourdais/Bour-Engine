#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "entity.h"
#include "components.h"

typedef struct TransformStorage {
    EntityId *entities;
    TransformComponent *components;
    size_t count;
    size_t capacity;
} TransformStorage;

void transform_storage_init(TransformStorage *storage);
bool transform_storage_add(TransformStorage *storage, EntityId entity, TransformComponent component);
TransformComponent *transform_storage_get(TransformStorage *storage, EntityId entity);
bool transform_storage_remove(TransformStorage *storage, EntityId entity);
void transform_storage_shutdown(TransformStorage *storage);
