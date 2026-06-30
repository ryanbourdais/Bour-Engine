#include "instancedModel.h"

#include <stdlib.h>

int instanced_model_init(InstancedModel *instances)
{
    instances->transforms = malloc(16 * sizeof(mat4));

    if (instances->transforms == NULL)
    {
        instances->count = 0;
        instances->capacity = 0;
        instances->instance_vbo = 0;
        return 1;
    }

    instances->count = 0;
    instances->capacity = 16;

    glGenBuffers(1, &instances->instance_vbo);
    return 0;
}

bool instanced_model_add_transform(InstancedModel *instances, mat4 transform)
{
    if (instances->count == instances->capacity)
    {
        size_t new_capacity = instances->capacity * 2;

        mat4 *new_transforms = realloc(instances->transforms, new_capacity * sizeof(mat4));

        if (new_transforms == NULL)
        {
            return false;
        }

        instances->transforms = new_transforms;
        instances->capacity = new_capacity;
    }

    glm_mat4_copy(transform, instances->transforms[instances->count]);
    instances->count++;

    return true;
}

void instanced_model_upload_transforms(InstancedModel *instances)
{
    glBindBuffer(GL_ARRAY_BUFFER, instances->instance_vbo);

    glBufferData(GL_ARRAY_BUFFER, instances->count * sizeof(mat4), instances->transforms, GL_STATIC_DRAW);
}

void instanced_model_free(InstancedModel *instances)
{
    if (instances == NULL)
    {
        return;
    }

    if (instances->instance_vbo)
    {
        glDeleteBuffers(1, &instances->instance_vbo);
    }

    free(instances->transforms);

    instances->transforms = NULL;
    instances->count = 0;
    instances->capacity = 0;
    instances->instance_vbo = 0;
}