#pragma once

#include "engine_runtime.h"

#include "../renderer/camera.h"
#include "../renderer/renderer.h"
#include "../scene/scene.h"

struct EngineRuntime {
    Camera camera;
    Renderer *renderer;
    Scene scene;
    char current_scene_path[ENGINE_RUNTIME_SCENE_PATH_MAX_LENGTH];
    bool has_current_scene_path;
};
