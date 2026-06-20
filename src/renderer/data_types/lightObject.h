#pragma once
#include <glad/glad.h>
#include <cglm/struct.h>
#include <stddef.h>
#include "renderObject.h"

// typedef struct LightObject
// {
//     vec3s position;
//     vec3s color;
//     bool has_visual;
//     RenderObject visual;
// } LightObject;

typedef struct LightColor
{
    vec3s ambient;
    vec3s diffuse;
    vec3s specular;
} LightColor;

typedef struct PointLight
{
    vec3s position;
    LightColor color;

    float constant;
    float linear;
    float quadratic;

    bool has_visual;
    RenderObject visual;
} PointLight;

typedef struct PointLightUniforms
{
    GLint position;

    GLint ambient;
    GLint diffuse;
    GLint specular;

    GLint constant;
    GLint linear;
    GLint quadratic;
} PointLightUniforms;

typedef struct DirectionalLight
{
    vec3s direction;
    LightColor color;
} DirectionalLight;

typedef struct DirectionalLightUniforms
{
    GLint direction;
    GLint ambient;
    GLint diffuse;
    GLint specular;
} DirectionalLightUniforms;

typedef struct SpotLight
{
    vec3s position;
    vec3s direction;
    LightColor color;

    float constant;
    float linear;
    float quadratic;

    float inner_cutoff_degrees;
    float outer_cuttoff_degrees;
} SpotLight;

typedef struct SpotLightUniforms
{
    GLint position;
    GLint direction;

    GLint ambient;
    GLint diffuse;
    GLint specular;

    GLint constant;
    GLint linear;
    GLint quadratic;

    GLint inner_cuttoff;
    GLint outer_cutoff;
} SpotLightUniforms;

// void point_light_object_init(LightObject *light, vec3s position, vec3s color);
// void light_object_init_with_visual(LightObject *light, vec3s position, vec3s color, RenderObject visual);
void directional_light_init(DirectionalLight *light, vec3s direction, LightColor color);
void point_light_init(PointLight *light, vec3s position, LightColor color, float constant, float linear, float quadratic);
void point_light_set_visual(PointLight* light, RenderObject visual);
void spot_light_init(SpotLight *light, vec3s position, vec3s direction, LightColor color, float constant, float linear, float quadratic, float inner_cutoff_degrees, float outer_cutoff_degrees);
