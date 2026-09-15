#include "programmable_mesh_resources.h"
#include "mesh.h"
#include "vertex.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static ProgrammableMeshResource *find_resource(
    ProgrammableMeshResources *resources,
    ProgrammableMeshId id
)
{
    if (resources == NULL || id == PROGRAMMABLE_MESH_ID_INVALID)
    { 
        return NULL;
    }

    for (size_t i = 0; i < resources->resource_count; i++)
    {
        ProgrammableMeshResource *resource = &resources->resources[i];
        
        if (resource->in_use && resource->id == id)
        {
            return resource;
        }
    }

    return NULL;
}

static const ProgrammableMeshResource *find_resource_const(
    const ProgrammableMeshResources *resources,
    ProgrammableMeshId id
)
{
    if (resources == NULL || id == PROGRAMMABLE_MESH_ID_INVALID)
    {
        return NULL;
    }

    for (size_t i = 0; i < resources->resource_count; i++)
    {
        const ProgrammableMeshResource *resource =
            &resources->resources[i];

        if (resource->in_use && resource->id == id)
        {
            return resource;
        }
    }

    return NULL;
}

static bool reserve_resources(
    ProgrammableMeshResources *resources,
    size_t minimum_capacity
)
{
    if (resources == NULL )
    {
        return false;
    }

    if (resources->resource_capacity >= minimum_capacity)
    {
        return true;
    }

    size_t new_capacity =
        resources->resource_capacity == 0 ? 4 : resources->resource_capacity;

    while (new_capacity < minimum_capacity) 
    {
        if (new_capacity > SIZE_MAX / 2)
        {
            return false;
        }

        new_capacity *= 2;
    }

    if (new_capacity > SIZE_MAX / sizeof(*resources->resources))
    {
        return false;
    }

    ProgrammableMeshResource *new_resources = realloc(
        resources->resources,
        new_capacity * sizeof(*new_resources)
    );

    if (new_resources == NULL)
    {
        return false;
    }

    resources->resources = new_resources;
    resources->resource_capacity = new_capacity;

    return true;
}

void programmable_mesh_resources_init(
    ProgrammableMeshResources *resources
)
{
    if (resources == NULL)
    {
        return;
    }

    resources->resources = NULL;
    resources->resource_count = 0;
    resources->resource_capacity = 0;
}

void programmable_mesh_resources_begin_frame(
    ProgrammableMeshResources *resources
)
{
    if (resources == NULL)
    {
        return;
    }

    for (size_t i = 0; i < resources->resource_count; i++)
    {
        resources->resources[i].seen_this_frame = false;
    }
}

static Vertex *create_renderer_vertices(
    const ProgrammableMesh *source
)
{
    if (source == NULL ||
        source->vertices == NULL ||
        source->vertex_count == 0 ||
        source->vertex_count > SIZE_MAX / sizeof(Vertex))
    {
        return NULL;
    }

    Vertex *vertices = malloc(
        source->vertex_count * sizeof(*vertices)
    );

    if (vertices == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < source->vertex_count; i++)
    {
        vertices[i] = (Vertex) {
            .position = source->vertices[i].position,
            .color = source->vertices[i].color,
            .uv = source->vertices[i].uv,
            .normal = source->vertices[i].normal,
        };
    }

    return vertices;
}

 bool programmable_mesh_resources_sync(
    ProgrammableMeshResources *resources,
    ProgrammableMeshId id,
    ProgrammableMesh *source
)
{
    if (resources == NULL ||
        source == NULL ||
        id == PROGRAMMABLE_MESH_ID_INVALID ||
        source->vertices == NULL ||
        source->indices == NULL ||
        source->vertex_count == 0 ||
        source->index_count == 0 ||
        source->vertex_count > INT_MAX ||
        source->index_count > INT_MAX)
    {
        return false;
    }

    ProgrammableMeshResource *resource =
        find_resource(resources, id);

    bool is_new = resource == NULL;

    if (is_new)
    {
        if (resources->resource_count == SIZE_MAX ||
            !reserve_resources(
                resources, 
                resources->resource_count + 1
            ))
        {
            return false;
        }

        resource = &resources->resources[resources->resource_count];
        resource->id = id;
        resource->in_use = true;
        resource->seen_this_frame = true;
        mesh_init(&resource->mesh);
    }
    else
    {
       resource->seen_this_frame = true; 
    }

    if (!is_new && !source->dirty)
    {
        return true;
    }

    Vertex *vertices = create_renderer_vertices(source);

    if (vertices == NULL)
    {
        return false;
    }

    Mesh uploaded_mesh;
    mesh_init(&uploaded_mesh);

    int upload_result = create_mesh_from_vertices(
        &uploaded_mesh, 
        vertices, 
        source->vertex_count, 
        source->indices, 
        (GLsizei)source->index_count
    );

    free(vertices);

    if (upload_result != 0)
    {
        mesh_free(&uploaded_mesh);

        if (is_new)
        {
            resource->id = PROGRAMMABLE_MESH_ID_INVALID;
            resource->in_use = false;
            resource->seen_this_frame = false;
        }
        return false;
    }

    if (!is_new)
    {
        mesh_free(&resource->mesh);
    }

    resource->mesh = uploaded_mesh;

    if (is_new)
    {
        resources->resource_count++;
    }

    programmable_mesh_mark_clean(source);
    return true;
}

const Mesh *programmable_mesh_resources_get(
    const ProgrammableMeshResources *resources,
    ProgrammableMeshId id
)
{
    const ProgrammableMeshResource *resource =
        find_resource_const(resources, id);

    if (resource == NULL)
    {
        return NULL;
    }

    return &resource->mesh;
}

void programmable_mesh_resources_end_frame(
    ProgrammableMeshResources *resources
)
{
    if (resources == NULL)
    {
        return;
    }

    size_t write_index = 0;

    for (size_t read_index = 0;
         read_index < resources->resource_count;
         read_index++)
    {
        ProgrammableMeshResource *resource =
            &resources->resources[read_index];

        if (!resource->in_use)
        {
            continue;
        }

        if (!resource->seen_this_frame)
        {
            mesh_free(&resource->mesh);
            continue;
        }

        if (write_index != read_index)
        {
            resources->resources[write_index] = *resource;
        }

        write_index++;
    }

    resources->resource_count = write_index;
}

void programmable_mesh_resources_free(
    ProgrammableMeshResources *resources
)
{
    if (resources == NULL)
    {
        return;
    }

    for (size_t i = 0; i < resources->resource_count; i++)
    {
        if (resources->resources[i].in_use)
        {
            mesh_free(&resources->resources[i].mesh);
        }
    }

    free(resources->resources);
    programmable_mesh_resources_init(resources);
}

