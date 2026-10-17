#include "model_resources.h"
#include "model.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static ModelResource *model_resources_find(
    ModelResources *resources,
    const char *path
)
{
   if (resources == NULL || path == NULL)
   {
       return NULL;
   }

   for (size_t index = 0; index < resources->resource_count; index++)
   {
        ModelResource *resource = &resources->resources[index];

        if (strcmp(resource->path, path) == 0)
        {
            return resource;
        }
   }

   return NULL;
}

static const ModelResource *model_resources_find_const(
    const ModelResources *resources,
    const char *path
)
{
    if (resources == NULL || path == NULL)
    {
        return NULL;
    }

    for (size_t index = 0; index < resources->resource_count; index++)
    {
        const ModelResource *resource =
            &resources->resources[index];

        if (strcmp(resource->path, path) == 0)
        {
            return resource;
        }
    }

    return NULL;
}

void model_resources_init(
    ModelResources *resources
)
{
    if (resources == NULL)
    {
        return;
    }

    *resources = (ModelResources){0};
}

const Model *model_resources_get(
    const ModelResources *resources,
    const char *path
)
{
    const ModelResource *resource =
        model_resources_find_const(resources, path);

    if (resource == NULL || !resource->loaded)
    {
        return NULL;
    }

    return &resource->model;
}

Model *model_resources_get_or_load(
    ModelResources *resources,
    const char *path
)
{
    if (resources == NULL || path == NULL || path[0] == '\0')
    {
        return NULL;
    }

    ModelResource *existing = model_resources_find(resources, path);

    if (existing != NULL)
    {
        return existing->loaded ? &existing->model : NULL;
    }
    if (strlen(path) >= RENDERER_MODEL_RESOURCE_PATH_MAX_LENGTH)
    {
        fprintf(stderr, "Model resource path is too long: %s\n", path);
        return NULL;
    }

    if (resources->resource_count >= RENDERER_MODEL_RESOURCE_MAX_COUNT)
    {
        fprintf(
            stderr, 
            "Model resource cache is full; cannot load: %s\n",
            path
        );
        return NULL;
    }

    ModelResource *resource =
        &resources->resources[resources->resource_count++];

    snprintf(resource->path, sizeof(resource->path), "%s", path);

    if (model_load_gltf(&resource->model, resource->path) != 0)
    {
        fprintf(stderr, "Failed to load model resource: %s\n", path);
        return NULL;
    }

    resource->loaded = true;

    return &resource->model;
}

void model_resources_free(
    ModelResources *resources
)
{
    if (resources == NULL)
    {
        return;
    }

    for (size_t index = 0; index < resources->resource_count; index++)
    {
        ModelResource *resource = &resources->resources[index];

        if (resource->loaded)
        {
            model_free(&resource->model);
        }
    }

    model_resources_init(resources);
}





