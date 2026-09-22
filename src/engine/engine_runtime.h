#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EngineRuntime EngineRuntime;

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
