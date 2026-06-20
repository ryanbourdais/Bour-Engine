#pragma once
#include <glad/glad.h>
#include <cglm/struct.h>
#include <stddef.h>
#include "renderObject.h"

typedef struct LightObject
{
    vec3s position;
    vec3s color;
    bool has_visual;
    RenderObject visual;
} LightObject;

typedef struct LightColor
{
    vec3s ambient;
    vec3s diffuse;
    vec3s specular;
} LightColor;

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

void point_light_object_init(LightObject *light, vec3s position, vec3s color);
void light_object_init_with_visual(LightObject *light, vec3s position, vec3s color, RenderObject visual);
void directional_light_init(DirectionalLight *light, vec3s direction, LightColor color);
