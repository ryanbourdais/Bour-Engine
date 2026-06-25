
#include "lightObject.h"
#include <math.h>
#include <stdio.h>

void upload_directional_light(DirectionalLight *light, DirectionalLightUniforms *uniform)
{
    glUniform3fv(
        uniform->direction,
        1,
        light->direction.raw);

    glUniform3fv(
        uniform->ambient,
        1,
        light->color.ambient.raw);
    glUniform3fv(
        uniform->diffuse,
        1,
        light->color.diffuse.raw);
    glUniform3fv(
        uniform->specular,
        1,
        light->color.specular.raw);
}

void directional_light_init(DirectionalLight *light, vec3s direction, LightColor color)
{
    light->direction = direction;
    light->color = color;
}

void directional_light_uniforms_init(DirectionalLightUniforms *uniforms, GLuint shader_program)
{
    uniforms->direction =
        glGetUniformLocation(shader_program, "directionalLight.direction");

    uniforms->ambient =
        glGetUniformLocation(shader_program, "directionalLight.ambient");

    uniforms->diffuse =
        glGetUniformLocation(shader_program, "directionalLight.diffuse");

    uniforms->specular =
        glGetUniformLocation(shader_program, "directionalLight.specular");
}

void upload_point_light_collection(PointLightCollection *point_lights, PointLightUniforms *uniforms, GLint point_light_location)
{
    glUniform1i(point_light_location, (GLint)point_lights->count);
    for (size_t i = 0; i < point_lights->count; i++)
    {
        upload_point_light(&point_lights->items[i], &uniforms[i]);
    }
}

void upload_point_light(PointLight *light, PointLightUniforms *uniforms)
{
    glUniform3fv(
        uniforms->position,
        1,
        light->position.raw);
    glUniform3fv(
        uniforms->ambient,
        1,
        light->color.ambient.raw);
    glUniform3fv(
        uniforms->diffuse,
        1,
        light->color.diffuse.raw);
    glUniform3fv(
        uniforms->specular,
        1,
        light->color.specular.raw);
    glUniform1f(
        uniforms->constant,
        light->constant);
    glUniform1f(
        uniforms->linear,
        light->linear);
    glUniform1f(
        uniforms->quadratic,
        light->quadratic);
}

void point_light_init(PointLight *light, vec3s position, LightColor color, float constant, float linear, float quadratic)
{
    light->position = position;
    light->color = color;
    light->constant = constant;
    light->linear = linear;
    light->quadratic = quadratic;
    light->has_visual = false;
}

void point_light_set_visual(PointLight *light, RenderObject visual)
{
    light->visual = visual;
    light->has_visual = true;
}

void point_light_collection_init(PointLightCollection *lights)
{
    lights->count = 0;
}

void point_light_uniforms_init(PointLightUniforms *uniforms, GLuint shader_program, size_t index)
{
    char name[64];

    snprintf(name, sizeof(name), "pointLights[%zu].position", index);
    uniforms->position = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "pointLights[%zu].ambient", index);
    uniforms->ambient = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "pointLights[%zu].diffuse", index);
    uniforms->diffuse = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "pointLights[%zu].specular", index);
    uniforms->specular = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "pointLights[%zu].constant", index);
    uniforms->constant = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "pointLights[%zu].linear", index);
    uniforms->linear = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "pointLights[%zu].quadratic", index);
    uniforms->quadratic = glGetUniformLocation(shader_program, name);
}
bool point_light_collection_add(PointLightCollection *lights, PointLight light)
{
    if(lights->count >= MAX_SHADER_POINT_LIGHTS)
    {
        return false;
    }

    lights->items[lights->count] = light;
    lights->count++;

    return true;
}

void upload_spot_light_collection(SpotLightCollection *spot_lights, SpotLightUniforms *uniforms, GLint spot_light_count_location)
{
    glUniform1i(spot_light_count_location, (GLint)spot_lights->count);
    for (size_t i = 0; i < spot_lights->count; i++)
    {
        upload_spot_light(&spot_lights->items[i], &uniforms[i]);
    }
}

void upload_spot_light(SpotLight *spot, SpotLightUniforms *uniform)
{
    glUniform3fv(uniform->position, 1, spot->position.raw);
    glUniform3fv(uniform->direction, 1, spot->direction.raw);

    glUniform3fv(uniform->ambient, 1, spot->color.ambient.raw);
    glUniform3fv(uniform->diffuse, 1, spot->color.diffuse.raw);
    glUniform3fv(uniform->specular, 1, spot->color.specular.raw);

    glUniform1f(uniform->constant, spot->constant);
    glUniform1f(uniform->linear, spot->linear);
    glUniform1f(uniform->quadratic, spot->quadratic);

    glUniform1f(
        uniform->inner_cutoff,
        cosf(glm_rad(spot->inner_cutoff_degrees)));

    glUniform1f(
        uniform->outer_cutoff,
        cosf(glm_rad(spot->outer_cutoff_degrees)));
}

void spot_light_init(SpotLight *light, vec3s position, vec3s direction, LightColor color, float constant, float linear, float quadratic, float inner_cutoff_degrees, float outer_cutoff_degrees)
{
    light->position = position;
    light->direction = direction;
    light->color = color;

    light->constant = constant;
    light->linear = linear;
    light->quadratic = quadratic;

    light->inner_cutoff_degrees = inner_cutoff_degrees;
    light->outer_cutoff_degrees = outer_cutoff_degrees;
}

void spot_light_collection_init(SpotLightCollection *lights)
{
    lights->count = 0;
}

void spot_light_uniforms_init(SpotLightUniforms *uniforms, GLuint shader_program, size_t index)
{
    char name[64];

    snprintf(name, sizeof(name), "spotLights[%zu].position", index);
    uniforms->position = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].direction", index);
    uniforms->direction = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].ambient", index);
    uniforms->ambient = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].diffuse", index);
    uniforms->diffuse = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].specular", index);
    uniforms->specular = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].constant", index);
    uniforms->constant = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].linear", index);
    uniforms->linear = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].quadratic", index);
    uniforms->quadratic = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].innerCutoff", index);
    uniforms->inner_cutoff = glGetUniformLocation(shader_program, name);

    snprintf(name, sizeof(name), "spotLights[%zu].outerCutoff", index);
    uniforms->outer_cutoff = glGetUniformLocation(shader_program, name);
}

bool spot_light_collection_add(SpotLightCollection *lights, SpotLight light)
{
    if(lights->count >= MAX_SHADER_SPOT_LIGHTS)
    {
        return false;
    }

    lights->items[lights->count] = light;
    lights->count++;

    return true;
}