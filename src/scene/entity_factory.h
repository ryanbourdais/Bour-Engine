#pragma once

#include "scene.h"

EntityId scene_entity_factory_create_asset(
    Scene *scene,
    const char *name_value,
    const char *model_path,
    const TransformComponent *initial_transform
);

EntityId scene_entity_factory_create_primitive(
    Scene *scene,
    const char *name_value,
    BuiltinPrimitiveType primitive_type,
    const TransformComponent *initial_transform
);

EntityId scene_entity_factory_create_programmable_plane(
    Scene *scene,
    const char *name_value,
    float width,
    float depth,
    const TransformComponent *initial_transform
);
