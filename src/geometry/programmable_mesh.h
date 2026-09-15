#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <cglm/struct.h>

typedef struct ProgrammableMeshVertex {
    vec3s position;
    vec3s color;
    vec2s uv;
    vec3s normal;
} ProgrammableMeshVertex;

typedef enum ProgrammableMeshType {
    PROGRAMMABLE_MESH_TYPE_NONE = 0,
    PROGRAMMABLE_MESH_TYPE_PLANE
} ProgrammableMeshType;

typedef struct ProgrammableMesh {
    ProgrammableMeshType type;

    float plane_width;
    float plane_depth;

    ProgrammableMeshVertex *vertices;
    size_t vertex_count;

    unsigned int *indices;
    size_t index_count;

    bool dirty;
} ProgrammableMesh;

typedef unsigned int ProgrammableMeshId;

#define PROGRAMMABLE_MESH_ID_INVALID 0U

typedef struct ProgrammableMeshSlot {
    ProgrammableMeshId id;
    bool in_use;
    ProgrammableMesh mesh;
} ProgrammableMeshSlot;

typedef struct ProgrammableMeshCollection {
    ProgrammableMeshSlot *slots;
    size_t slot_count;
    size_t slot_capacity;
    ProgrammableMeshId next_id;
} ProgrammableMeshCollection;

void programmable_mesh_init(ProgrammableMesh *mesh);
void programmable_mesh_free(ProgrammableMesh *mesh);

bool programmable_mesh_create_plane(
    ProgrammableMesh *mesh,
    float width,
    float depth
);

ProgrammableMeshVertex *programmable_mesh_vertices_for_write(
    ProgrammableMesh *mesh,
    size_t *out_vertex_count
);

void programmable_mesh_mark_clean(ProgrammableMesh *mesh);

void programmable_mesh_collection_init(
    ProgrammableMeshCollection *collection
);

void programmable_mesh_collection_free(
    ProgrammableMeshCollection *collection
);

bool programmable_mesh_collection_create_plane(
    ProgrammableMeshCollection *collection,
    float width,
    float depth,
    ProgrammableMeshId *out_id
);

ProgrammableMesh *programmable_mesh_collection_get(
    ProgrammableMeshCollection *collection,
    ProgrammableMeshId id
);

const ProgrammableMesh *programmable_mesh_collection_get_const(
    const ProgrammableMeshCollection *collection,
    ProgrammableMeshId id 
);

bool programmable_mesh_collection_remove(
    ProgrammableMeshCollection *collection,
    ProgrammableMeshId id 
);
