#pragma once

#include "scene.h"

typedef enum SceneSaveResult {
    SCENE_SAVE_OK = 0,
    SCENE_SAVE_INVALID_ARGUMENT,
    SCENE_SAVE_FILE_OPEN_FAILED,
    SCENE_SAVE_WRITE_FAILED
} SceneSaveResult;

SceneSaveResult scene_save_to_file(const Scene *scene, const char *path);
