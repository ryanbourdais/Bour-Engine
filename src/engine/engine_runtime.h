#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EngineRuntime EngineRuntime;

#define ENGINE_RUNTIME_INVALID_ENTITY_ID 0
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

typedef struct EngineRuntimeRenderStats {
    size_t renderable_count;

    size_t mesh_count;
    size_t vertex_count;
    size_t triangle_count;
    size_t texture_count;
    
    size_t submitted_draw_count;
    size_t submitted_mesh_count;
    size_t submitted_vertex_count;
    size_t submitted_triangle_count;
    size_t missing_model_count;

    int viewport_width;
    int viewport_height;
    size_t render_target_resize_count;
    size_t render_target_noop_count;
    size_t zero_size_viewport_count;
} EngineRuntimeRenderStats;

#define ENGINE_RUNTIME_SCENE_PATH_MAX_LENGTH 256
typedef enum EngineRuntimePrimitiveType {
    ENGINE_RUNTIME_PRIMITIVE_CUBE = 0,
    ENGINE_RUNTIME_PRIMITIVE_PLANE,
    ENGINE_RUNTIME_PRIMITIVE_QUAD,
    ENGINE_RUNTIME_PRIMITIVE_UV_SPHERE,
    ENGINE_RUNTIME_PRIMITIVE_CYLINDER,
} EngineRuntimePrimitiveType;

typedef enum EngineRuntimeCommandType {
    ENGINE_RUNTIME_COMMAND_CREATE_EMPTY_ENTITY = 0,
    ENGINE_RUNTIME_COMMAND_CREATE_RENDERABLE_ENTITY,
    ENGINE_RUNTIME_COMMAND_CREATE_PRIMITIVE_ENTITY,
    ENGINE_RUNTIME_COMMAND_CREATE_PROGRAMMABLE_PLANE,
    ENGINE_RUNTIME_COMMAND_DELETE_ENTITY,
    ENGINE_RUNTIME_COMMAND_DUPLICATE_ENTITY,
    ENGINE_RUNTIME_COMMAND_RENAME_ENTITY,
    ENGINE_RUNTIME_COMMAND_SET_TRANSFORM,
    ENGINE_RUNTIME_COMMAND_SET_LIGHT,
    ENGINE_RUNTIME_COMMAND_SAVE_SCENE,
    ENGINE_RUNTIME_COMMAND_LOAD_SCENE,
} EngineRuntimeCommandType; 

typedef enum EngineRuntimeCommandResult {
    ENGINE_RUNTIME_COMMAND_OK = 0,
    ENGINE_RUNTIME_COMMAND_INVALID_ARGUMENT,
    ENGINE_RUNTIME_COMMAND_ENTITY_NOT_FOUND,
    ENGINE_RUNTIME_COMMAND_OPERATION_FAILED,
    ENGINE_RUNTIME_COMMAND_SCENE_IO_FAILED,
} EngineRuntimeCommandResult;

typedef struct EngineRuntimeCommand {
    EngineRuntimeCommandType type;
    uint32_t entity_id;

    char name[ENGINE_RUNTIME_ENTITY_NAME_MAX_LENGTH];
    char scene_path[ENGINE_RUNTIME_SCENE_PATH_MAX_LENGTH];

    EngineRuntimePrimitiveType primitive_type;
    float programmable_plane_width;
    float programmable_plane_depth;
    
    float position[3];
    float rotation[3];
    float scale[3];

    float light_ambient[3];
    float light_diffuse[3];
    float light_specular[3];
    float light_direction[3];
    float light_position[3];
} EngineRuntimeCommand;

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
    bool simulation_update_enabled;
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

EngineRuntime *engine_runtime_create_preview(
    const EngineRuntime *authoring_runtime,
    int framebuffer_width,
    int framebuffer_height
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

EngineRuntimeRenderStats engine_runtime_get_render_stats(
    EngineRuntime *runtime
);

EngineRuntimeCommandResult engine_runtime_execute_command(
    EngineRuntime *runtime,
    const EngineRuntimeCommand *command,
    uint32_t *out_affected_entity_id
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
