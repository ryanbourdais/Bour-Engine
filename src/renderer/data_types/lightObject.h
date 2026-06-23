#pragma once
#include <glad/glad.h>
#include <cglm/struct.h>
#include <stddef.h>
#include <stdbool.h>
#include "renderObject.h"

typedef struct LightColor
{
    vec3s ambient;
    vec3s diffuse;
    vec3s specular;
} LightColor;

#define MAX_POINT_LIGHTS 4
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

typedef struct PointLightCollection
{
    PointLight items[MAX_POINT_LIGHTS];
    size_t count;
} PointLightCollection;

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

#define MAX_SPOT_LIGHTS 4
typedef struct SpotLight
{
    vec3s position;
    vec3s direction;
    LightColor color;

    float constant;
    float linear;
    float quadratic;

    float inner_cutoff_degrees;
    float outer_cutoff_degrees;
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

    GLint inner_cutoff;
    GLint outer_cutoff;
} SpotLightUniforms;

typedef struct SpotLightCollection
{
    SpotLight items[MAX_SPOT_LIGHTS];
    size_t count;
} SpotLightCollection;

void upload_directional_light(DirectionalLight *light, DirectionalLightUniforms *uniform);
void directional_light_init(DirectionalLight *light, vec3s direction, LightColor color);
void directional_light_uniforms_init(DirectionalLightUniforms uniforms, GLuint shader_program);
void upload_point_light_collection(PointLightCollection *point_lights, PointLightUniforms *uniforms, GLint point_light_location);
void upload_point_light(PointLight *light, PointLightUniforms *uniforms);
void point_light_init(PointLight *light, vec3s position, LightColor color, float constant, float linear, float quadratic);
void point_light_set_visual(PointLight* light, RenderObject visual);
void point_light_collection_init(PointLightCollection *lights);
void spot_light_uniforms_init(SpotLightUniforms *uniforms, GLuint shader_program, int index);
bool point_light_collection_add(PointLightCollection *lights, PointLight light);
void upload_spot_light_collection(SpotLightCollection *spot_lights, SpotLightUniforms *uniforms, GLint spot_light_count_location);
void upload_spot_light(SpotLight *spot, SpotLightUniforms *uniform);
void spot_light_init(SpotLight *light, vec3s position, vec3s direction, LightColor color, float constant, float linear, float quadratic, float inner_cutoff_degrees, float outer_cutoff_degrees);
void spot_light_collection_init(SpotLightCollection *lights);
bool spot_light_collection_add(SpotLightCollection *lights, SpotLight light);
