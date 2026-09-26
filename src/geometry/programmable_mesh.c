#include "programmable_mesh.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void programmable_mesh_init(ProgrammableMesh *mesh)
{
    if (mesh == NULL)
    {
        return;
    }

    mesh->vertices = NULL;
    mesh->vertex_count = 0;
    mesh->indices = NULL;
    mesh->index_count = 0;
    mesh->type = PROGRAMMABLE_MESH_TYPE_NONE;
    mesh->plane_width = 0.0f;
    mesh->plane_depth = 0.0f;
    mesh->dirty = false;
}

void programmable_mesh_free(ProgrammableMesh *mesh)
{
    if (mesh == NULL)
    {
        return;
    }

    free(mesh->vertices);
    free(mesh->indices);

    programmable_mesh_init(mesh);
}

bool programmable_mesh_create_plane(
    ProgrammableMesh *mesh, 
    float width, 
    float depth
)
{
    if (mesh == NULL || width <= 0.0f || depth <= 0.0f)
    {
        return false;
    }

    const size_t vertex_count = 4;
    const size_t index_count = 6;
    const float half_width = width * 0.5f;
    const float half_depth = depth * 0.5f;

    ProgrammableMeshVertex *vertices =
        malloc(vertex_count * sizeof(*vertices));
    unsigned int *indices =
        malloc(index_count * sizeof(*indices));

    if (vertices == NULL || indices == NULL)
    {
        free(vertices);
        free(indices);
        return false;
    }

    vertices[0] = (ProgrammableMeshVertex) {
        .position = {{-half_width, 0.0f, -half_depth}},
        .color = {{1.0f, 1.0f, 1.0f}},
        .uv = {{0.0f, 0.0f}},
        .normal = {{0.0f, 1.0f, 0.0f}},
    };

    vertices[1] = (ProgrammableMeshVertex) {
        .position = {{ half_width, 0.0f, -half_depth}},
        .color = {{1.0f, 1.0f, 1.0f}},
        .uv = {{1.0f, 0.0f}},
        .normal = {{0.0f, 1.0f, 0.0f}},
    };

    vertices[2] = (ProgrammableMeshVertex) {
        .position = {{half_width, 0.0f, half_depth}},
        .color = {{1.0f, 1.0f, 1.0f}},
        .uv = {{1.0f, 1.0f}},
        .normal = {{0.0f, 1.0f, 0.0f}},
    };
    
    vertices[3] = (ProgrammableMeshVertex) {
        .position = {{-half_width, 0.0f, half_depth}},
        .color = {{1.0f, 1.0f, 1.0f}},
        .uv = {{0.0f, 1.0f}},
        .normal = {{0.0f, 1.0f, 0.0f}},
    };

    indices[0] = 0;
    indices[1] = 2;
    indices[2] = 1;
    indices[3] = 2;
    indices[4] = 0;
    indices[5] = 3;

    free(mesh->vertices);
    free(mesh->indices);

    mesh->vertices = vertices;
    mesh->vertex_count = vertex_count;
    mesh->indices = indices;
    mesh->index_count = index_count;
    mesh->type = PROGRAMMABLE_MESH_TYPE_PLANE;
    mesh->plane_width = width;
    mesh->plane_depth = depth;
    mesh->dirty = true;

    return true;
}

ProgrammableMeshVertex *programmable_mesh_vertices_for_write(
    ProgrammableMesh *mesh,
    size_t *out_vertex_count
)
{
    if (out_vertex_count != NULL)
    {
        *out_vertex_count = 0;
    }

    if (mesh == NULL ||
        mesh->vertices == NULL ||
        mesh->vertex_count == 0)
    {
        return NULL;
    }

    if (out_vertex_count != NULL)
    {
        *out_vertex_count = mesh->vertex_count;
    }

    mesh->dirty = true;
    return mesh->vertices;
}

void programmable_mesh_mark_clean(ProgrammableMesh *mesh)
{
    if (mesh == NULL)
    {
        return;
    }
    mesh->dirty = false;
}

void programmable_mesh_collection_init(
    ProgrammableMeshCollection *collection
)
{
    if (collection == NULL)
    {
        return;
    }

    collection->slots = NULL;
    collection->slot_count = 0;
    collection->slot_capacity = 0;
    collection->next_id = 1;
}

void programmable_mesh_collection_free(
    ProgrammableMeshCollection *collection
)
{
    if (collection == NULL)
    {
        return;
    }

    for (size_t i = 0; i < collection->slot_count; i++)
    {
        if (collection->slots[i].in_use)
        {
            programmable_mesh_free(&collection->slots[i].mesh);
        }
    }

    free(collection->slots);
    
    programmable_mesh_collection_init(collection);
}

static bool programmable_mesh_collection_reserve(
    ProgrammableMeshCollection *collection,
    size_t minimum_capacity
)
{
    if (collection == NULL)
    {
        return false;
    }
    
    if (collection->slot_capacity >= minimum_capacity)
    {
        return true;
    }

    size_t new_capacity =
        collection->slot_capacity == 0 ? 4 : collection->slot_capacity;

    while (new_capacity < minimum_capacity)
    {
        if (new_capacity > SIZE_MAX / 2)
        {
            return false;
        }

        new_capacity *= 2;
    }

    ProgrammableMeshSlot *new_slots = realloc(
        collection->slots,
        new_capacity * sizeof(*new_slots)
    );

    if (new_slots == NULL)
    {
        return false;
    }

    collection->slots = new_slots;
    collection->slot_capacity = new_capacity;

    return true;
}

static bool programmable_mesh_clone(
    ProgrammableMesh *out_mesh,
    const ProgrammableMesh *source
)
{
    if (out_mesh == NULL || source == NULL)
    {
        return false;
    }

    programmable_mesh_init(out_mesh);

    out_mesh->type = source->type;
    out_mesh->plane_width = source->plane_width;
    out_mesh->plane_depth = source->plane_depth;
    out_mesh->dirty = source->dirty;

    if (source->vertex_count > 0)
    {
        out_mesh->vertices = malloc(
            source->vertex_count * sizeof(*out_mesh->vertices)
        );

        if (out_mesh->vertices == NULL)
        {
            programmable_mesh_free(out_mesh);
            return false;
        }

        memcpy(
            out_mesh->vertices, 
            source->vertices, 
            source->vertex_count * sizeof(*out_mesh->vertices)
        );

        out_mesh->vertex_count = source->vertex_count;
    }

    if (source->index_count > 0)
    {
        out_mesh->indices = malloc(
            source->index_count * sizeof(*out_mesh->indices)
        );

        if (out_mesh->indices == NULL)
        {
            programmable_mesh_free(out_mesh);
            return false;
        }

        memcpy(
            out_mesh->indices,
            source->indices,
            source->index_count * sizeof(*out_mesh->indices)
        );

        out_mesh->index_count = source->index_count;
    }
    
    return true;
}

bool programmable_mesh_collection_clone(
    ProgrammableMeshCollection *out_collection,
    const ProgrammableMeshCollection *source
)
{
    if (out_collection == NULL ||
        source == NULL ||
        out_collection == source )
    {
        return false;
    }

    programmable_mesh_collection_init(out_collection);

    if (source->slot_count == 0)
    {
        out_collection->next_id = source->next_id;
        return true;
    }

    out_collection->slots = calloc(
        source->slot_count,
        sizeof(*out_collection->slots)
    );

    if (out_collection->slots == NULL)
    {
        return false;
    }

    out_collection->slot_count = source->slot_count;
    out_collection->slot_capacity = source->slot_count;
    out_collection->next_id = source->next_id;

    for (size_t index = 0; index < source->slot_count; index++)
    {
        const ProgrammableMeshSlot *source_slot =
            &source->slots[index];
        ProgrammableMeshSlot *out_slot =
            &out_collection->slots[index];

        out_slot->id = source_slot->id;
        out_slot->in_use = source_slot->in_use;
        programmable_mesh_init(&out_slot->mesh);

        if (out_slot->in_use &&
            !programmable_mesh_clone(
                &out_slot->mesh,
                &source_slot->mesh
            ))
        {
            programmable_mesh_collection_free(out_collection);
            return false;
        }
    }

    return true;
}

bool programmable_mesh_collection_create_plane(
    ProgrammableMeshCollection *collection,
    float width,
    float depth,
    ProgrammableMeshId *out_id
)
{
    if (out_id != NULL)
    {
        *out_id = PROGRAMMABLE_MESH_ID_INVALID;
    }

    if (collection == NULL ||
        collection->next_id == PROGRAMMABLE_MESH_ID_INVALID ||
        collection->slot_count == SIZE_MAX)
    {
        return false;
    }

    if (!programmable_mesh_collection_reserve(
            collection, 
            collection->slot_count + 1 ))
    {
        return false;
    }

    ProgrammableMeshSlot *slot =
        &collection->slots[collection->slot_count];

    slot->id = collection->next_id;
    slot->in_use = false;
    programmable_mesh_init(&slot->mesh);

    if (!programmable_mesh_create_plane(&slot->mesh, width, depth))
    {
        return false;
    }

    slot->in_use = true;
    collection->slot_count++;
    collection->next_id++;

    if (out_id != NULL)
    {
        *out_id = slot->id;
    }

    return true;
}

ProgrammableMesh *programmable_mesh_collection_get(
    ProgrammableMeshCollection *collection,
    ProgrammableMeshId id
)
{
    if (collection == NULL ||
        id == PROGRAMMABLE_MESH_ID_INVALID)
    {
        return NULL;
    }

    for (size_t i = 0; i < collection->slot_count; i++)
    {
        ProgrammableMeshSlot *slot = &collection->slots[i];

        if (slot->in_use && slot->id == id)
        {
            return &slot->mesh;
        }
    }
    return NULL;
}

const ProgrammableMesh *programmable_mesh_collection_get_const(
    const ProgrammableMeshCollection *collection,
    ProgrammableMeshId id 
)
{   
    if (collection == NULL || id == PROGRAMMABLE_MESH_ID_INVALID)
    {
        return NULL;
    }

    for (size_t i = 0; i < collection->slot_count; i++)
    {
        const ProgrammableMeshSlot *slot = &collection->slots[i];

        if (slot->in_use && slot->id == id)
        {
            return &slot->mesh;
        }
    }

    return NULL;
}

bool programmable_mesh_collection_remove(
    ProgrammableMeshCollection *collection,
    ProgrammableMeshId id
)
{
    if (collection == NULL ||
        id == PROGRAMMABLE_MESH_ID_INVALID )
    {
        return false;
    }

    for (size_t i = 0; i < collection->slot_count; i++)
    {
        ProgrammableMeshSlot *slot = &collection->slots[i];

        if (slot->in_use && slot->id == id)
        {
            programmable_mesh_free(&slot->mesh);
            slot->id = PROGRAMMABLE_MESH_ID_INVALID;
            slot->in_use = false;
            return true;
        }
    }

    return false;
}





