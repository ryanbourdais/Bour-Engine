#include "programmable_mesh.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

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





