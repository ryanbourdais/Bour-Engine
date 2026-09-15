#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "../../geometry/programmable_mesh.h"
#include "mesh.h"

typedef struct ProgrammableMeshResource {
    ProgrammableMeshId id;
    bool in_use;
    bool seen_this_frame;
    Mesh mesh;
} ProgrammableMeshResource;

typedef struct ProgrammableMeshResources {
    ProgrammableMeshResource *resources;
    size_t resource_count;
    size_t resource_capacity;
} ProgrammableMeshResources;

void programmable_mesh_resources_init(
    ProgrammableMeshResources *resources
);

void programmable_mesh_resources_begin_frame(
    ProgrammableMeshResources *resources
);

bool programmable_mesh_resources_sync(
    ProgrammableMeshResources *resources,
    ProgrammableMeshId id,
    ProgrammableMesh *source
);

const Mesh *programmable_mesh_resources_get(
    const ProgrammableMeshResources *resources,
    ProgrammableMeshId id
);

void programmable_mesh_resources_end_frame(
    ProgrammableMeshResources *resources
);

void programmable_mesh_resources_free(
    ProgrammableMeshResources *resources
);
