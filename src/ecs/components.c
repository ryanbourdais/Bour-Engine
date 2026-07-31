#include "components.h"

#include <cglm/struct.h>
#include <cglm/struct/affine.h>
#include <cglm/struct/mat4.h>
#include <cglm/util.h>

void transform_component_init(TransformComponent *transform)
{
    transform->position = (vec3s){{0.0f, 0.0f, 0.0f}};
    transform->rotation = (vec3s){{0.0f, 0.0f, 0.0f}};
    transform->scale    = (vec3s){{1.0f, 1.0f, 1.0f}};
}

void transform_component_set_position(TransformComponent *transform, vec3s position)
{
    transform->position = position;
}

void transform_component_set_rotation(TransformComponent *transform, vec3s rotation)
{
    transform->rotation = rotation;
}

void transform_component_set_scale(TransformComponent *transform, vec3s scale)
{
    transform->scale = scale;
}

mat4s transform_component_model_matrix(const TransformComponent *transform)
{
    mat4s model = glms_mat4_identity();

    model = glms_translate(model, transform->position);

    model = glms_rotate_x(model, glm_rad(transform->rotation.x));

    model = glms_rotate_y(model, glm_rad(transform->rotation.y));

    model = glms_rotate_z(model, glm_rad(transform->rotation.z));

    model = glms_scale(model, transform->scale);

    return model;
}
