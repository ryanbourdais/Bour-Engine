#pragma once

#include "scene.h"
#include "../ecs/components.h"
typedef enum SceneSaveResult {
    SCENE_SAVE_OK = 0,
    SCENE_SAVE_INVALID_ARGUMENT,
    SCENE_SAVE_FILE_OPEN_FAILED,
    SCENE_SAVE_WRITE_FAILED
} SceneSaveResult;

typedef enum SceneLoadResult {
    SCENE_LOAD_OK = 0,
    SCENE_LOAD_INVALID_ARGUMENT,
    SCENE_LOAD_FILE_OPEN_FAILED,
    SCENE_LOAD_FILE_READ_FAILED,
    SCENE_LOAD_PARSE_FAILED,
    SCENE_LOAD_INVALID_VERSION,
    SCENE_LOAD_INVALID_SCENE
} SceneLoadResult;

#define SCENE_PARSED_NAME_MAX_LENGTH ENTITY_NAME_MAX_LENGTH
#define SCENE_PARSED_PATH_MAX_LENGTH 256
typedef struct ParsedEntityV0 {
    int id;
    bool has_name;
    char name[SCENE_PARSED_NAME_MAX_LENGTH];
    bool has_transform;
    TransformComponent transform;
    bool has_camera;
    CameraComponent camera;
    bool has_skybox;
    char skybox_faces[6][SCENE_PARSED_PATH_MAX_LENGTH];
    bool has_mesh_renderer;
    char mesh_model_path[SCENE_PARSED_PATH_MAX_LENGTH];
} ParsedEntityV0;

typedef struct ParsedSceneV0 {
    int active_camera_entity;
    int active_skybox_entity;
    ParsedEntityV0 *entities;
    int entity_count;
} ParsedSceneV0;

typedef struct SceneParseSummary {
    int active_camera_entity;
    int active_skybox_entity;
    int *entity_ids;
    int entity_count;
} SceneParseSummary;

SceneSaveResult scene_save_to_file(const Scene *scene, const char *path);
SceneLoadResult scene_load_from_file(Scene *scene, const char *path);
