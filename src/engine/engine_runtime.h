#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EngineRuntime EngineRuntime;

#define ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH 64
#define ENGINE_RUNTIME_MAX_HIERARCHY_ITEMS 256

typedef struct EngineRuntimeHierarchyItem {
    uint32_t entity_id;
    char name[ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH];
} EngineRuntimeHierarchyItem;

typedef enum EngineRuntimeLightType {
    ENGINE_RUNTIME_LIGHT_NONE = 0,
    ENGINE_RUNTIME_LIGHT_DIRECTIONAL,
    ENGINE_RUNTIME_LIGHT_POINT,
    ENGINE_RUNTIME_LIGHT_SPOT,
} EngineRuntimeLightType;

typedef struct EngineRuntimeEntitySnapshot {
    uint32_t entity_id;
    char name[ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH];

    bool has_transform;
    float position[3];
    float rotation[3];
    float scale[3];

    bool has_mesh_renderer;
    bool is_programmable_mesh;
    uint32_t programmable_mesh_id;
    float programmable_plane_width;
    float programmable_plane_depth;
    bool programmable_mesh_dirty;

    bool has_camera;
    bool has_skybox;
    bool has_directional_light;
    bool has_point_light;
    bool has_spot_light;

    EngineRuntimeLightType light_type;
    float light_ambient[3];
    float light_diffuse[3];
    float light_specular[3];
    float light_direction[3];
    float light_position[3];
} EngineRuntimeEntitySnapshot;

typedef struct EngineRuntimeCreateInfo {
    const char *scene_path;
    int framebuffer_width;
    int framebuffer_height;
} EngineRuntimeCreateInfo;

typedef struct EngineRuntimeInput {
    float movement_x;
    float movement_y;
    float mouse_delta_x;
    float mouse_delta_y;
    bool camera_input_enabled;
} EngineRuntimeInput;

typedef enum EngineRuntimeRenderTargetType {
    ENGINE_RUNTIME_RENDER_TARGET_DEFAULT_FRAMEBUFFER = 0,
    ENGINE_RUNTIME_RENDER_TARGET_OFFSCREEN_TEXTURE,
} EngineRuntimeRenderTargetType;

typedef struct EngineRuntimeRenderTarget {
    int framebuffer_width;
    int framebuffer_height;
    EngineRuntimeRenderTargetType type;
} EngineRuntimeRenderTarget;

EngineRuntime *engine_runtime_create(
    const EngineRuntimeCreateInfo *create_info
);

void engine_runtime_destroy(EngineRuntime *runtime);

size_t engine_runtime_get_entity_count(
    const EngineRuntime *runtime
);

size_t engine_runtime_copy_hierarchy(
    const EngineRuntime *runtime,
    EngineRuntimeHierarchyItem *items,
    size_t item_capacity
);

bool engine_runtime_get_entity_snapshot(
    const EngineRuntime *runtime,
    uint32_t entity_id,
    EngineRuntimeEntitySnapshot *snapshot
);

void engine_runtime_update(
    EngineRuntime *runtime,
    double delta_time,
    const EngineRuntimeInput *input
);

void engine_runtime_render(
    EngineRuntime *runtime,
    const EngineRuntimeRenderTarget *target
);

uint32_t engine_runtime_get_resolved_texture(
    const EngineRuntime *runtime
);

#ifdef __cplusplus
}
#endif
