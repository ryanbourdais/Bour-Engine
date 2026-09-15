#include "primitive_mesh_resources.h"
#include "mesh.h"
#include "primitive_mesh.h"

#include <stddef.h>

void primitive_mesh_resources_init(
    PrimitiveMeshResources *resources
)
{
    if (resources == NULL)
    {
        return;
    }

    for (size_t i = 0; i < BUILTIN_PRIMITIVE_COUNT; i++)
    {
        mesh_init(&resources->meshes[i]);
        resources->loaded[i] = false;
    }
}

bool primitive_mesh_resources_upload(
    PrimitiveMeshResources *resources
)
{
    if (resources == NULL)
    {
        return false;
    }

    for (size_t i = 0; i < BUILTIN_PRIMITIVE_COUNT; i++)
    {
        if (resources->loaded[i])
        {
            continue;
        }

        PrimitiveMeshData data;

        if (!primitive_mesh_get_data((BuiltinPrimitiveType)i, &data))
        {
            primitive_mesh_resources_free(resources);
            return false;
        }
        
        if (create_mesh_from_vertices(
                    &resources->meshes[i],
                    data.vertices,
                    data.vertex_count,
                    data.indices,
                    data.index_count) != 0)
        {
            primitive_mesh_resources_free(resources);
            return false;
        }
        resources->loaded[i] = true;
    }
    return true;
}

const Mesh *primitive_mesh_resources_get(
    const PrimitiveMeshResources *resources,
    BuiltinPrimitiveType type
)
{
    if (resources == NULL)
    {
        return NULL;
    }

    if (type >= BUILTIN_PRIMITIVE_COUNT)
    {
        return NULL;
    }

    if (!resources->loaded[type])
    {
        return NULL;
    }

    return &resources->meshes[type];
}

void primitive_mesh_resources_free(
    PrimitiveMeshResources *resources
)
{
    if (resources == NULL)
    {
        return;
    }

    for (size_t i = 0; i < BUILTIN_PRIMITIVE_COUNT; i++)
    {
        if (resources->loaded[i])
        {
            mesh_free(&resources->meshes[i]);
        }

        resources->loaded[i] = false;
    }
}
