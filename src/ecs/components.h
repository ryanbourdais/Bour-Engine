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

typedef enum MeshSourceType {
    MESH_SOURCE_ASSET = 0,
    MESH_SOURCE_PRIMITIVE,
    MESH_SOURCE_PROGRAMMABLE,
} MeshSourceType;

typedef enum BuiltinPrimitiveType {
    BUILTIN_PRIMITIVE_CUBE = 0,
    BUILTIN_PRIMITIVE_PLANE,
    BUILTIN_PRIMITIVE_QUAD,
    BUILTIN_PRIMITIVE_UV_SPHERE,
    BUILTIN_PRIMITIVE_CYLINDER
} BuiltinPrimitiveType;

typedef struct MeshRendererComponent {
    MeshSourceType source_type;

    const char *model_path;

    BuiltinPrimitiveType primitive_type;

    unsigned int programmable_mesh_id;
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

typedef struct SkyboxComponent {
    const char *faces[6];
} SkyboxComponent;

void transform_component_init(TransformComponent *transform);
void transform_component_set_position(TransformComponent *transform, vec3s position);
void transform_component_set_rotation(TransformComponent *transform, vec3s rotation);
void transform_component_set_scale(TransformComponent *transform, vec3s scale);
mat4s transform_component_model_matrix(const TransformComponent *transform);
