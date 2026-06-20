#include "lightObject.h"

void point_light_object_init(LightObject *light, vec3s position, vec3s color)
{
    light->position = position;
    light->color = color;
    light->has_visual = false;
}
void light_object_init_with_visual(LightObject *light, vec3s position, vec3s color, RenderObject visual)
{
    light->position = position;
    light->color = color;
    light->has_visual = true;
    light->visual = visual;
}

void directional_light_init(DirectionalLight *light, vec3s direction, LightColor color)
{
    light->direction = direction;
    light->color = color;
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