#pragma once

#include "engine_runtime.h"

#include "../renderer/camera.h"
#include "../renderer/renderer.h"
#include "../scene/scene.h"

struct EngineRuntime {
    Camera camera;
    Renderer *renderer;
    Scene scene;
};
