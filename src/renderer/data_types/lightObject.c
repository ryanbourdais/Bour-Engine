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
