#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "model.h"

#define RENDERER_MODEL_RESOURCE_MAX_COUNT 64
#define RENDERER_MODEL_RESOURCE_PATH_MAX_LENGTH 256

typedef struct ModelResource {
    char path[RENDERER_MODEL_RESOURCE_PATH_MAX_LENGTH];
    bool loaded;
    Model model;
} ModelResource;

typedef struct ModelResources {
    ModelResource resources[RENDERER_MODEL_RESOURCE_MAX_COUNT];
    size_t resource_count;
} ModelResources;

void model_resources_init(
    ModelResources *resources
);

const Model *model_resources_get(
    const ModelResources *resources,
    const char *path
);

Model *model_resources_get_or_load(
    ModelResources *resources,
    const char *path,
    const char *project_root
);

void model_resources_free(
    ModelResources *resources
);
