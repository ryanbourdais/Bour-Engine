#pragma once

#include <cglm/struct.h>
#include "../renderer/data_types/lightObject.h"

typedef struct TransformComponent {
    vec3s position;
    vec3s rotation;
    vec3s scale;
} TransformComponent;

#define ENTITY_NAME_MAX_LENGTH 64
typedef struct NameComponent {
    char value[ENTITY_NAME_MAX_LENGTH];
} NameComponent;

typedef struct MeshRendererComponent {
    const char *model_path;
} MeshRendererComponent;

typedef struct DirectionalLightComponent {
    DirectionalLight light;
} DirectionalLightComponent;

typedef struct PointLightComponent {
    PointLight light;
} PointLightComponent;

typedef struct SpotLightComponent {
    SpotLight light;
} SpotLightComponent;

typedef struct CameraComponent {
    float fov;
    float near_clip;
    float far_clip;
} CameraComponent;