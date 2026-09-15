
#include "primitive_mesh.h"
#include "vertex.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

static const Vertex cube_vertices[] = {
    {{{-0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{ 0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{ 0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{-0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}},

    {{{ 0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}},
    {{{-0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, -1.0f}}},
    {{{-0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}},
    {{{ 0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}},

    {{{-0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{-1.0f, 0.0f, 0.0f}}},
    {{{-0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{-1.0f, 0.0f, 0.0f}}},
    {{{-0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{-1.0f, 0.0f, 0.0f}}},
    {{{-0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{-1.0f, 0.0f, 0.0f}}},

    {{{ 0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}}},
    {{{ 0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}}},
    {{{ 0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{1.0f, 0.0f, 0.0f}}},
    {{{ 0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{1.0f, 0.0f, 0.0f}}},

    {{{-0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{ 0.5f,  0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{ 0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{-0.5f,  0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}}},

    {{{-0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, -1.0f, 0.0f}}},
    {{{ 0.5f, -0.5f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, -1.0f, 0.0f}}},
    {{{ 0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, -1.0f, 0.0f}}},
    {{{-0.5f, -0.5f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, -1.0f, 0.0f}}},
};

static const unsigned int cube_indices[] = {
    0,  1,  2,  2,  3,  0,
    4,  5,  6,  6,  7,  4,
    8,  9, 10, 10, 11,  8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
};

static const Vertex plane_vertices[] = {
    {{{-0.5f, 0.0f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{ 0.5f, 0.0f, -0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{ 0.5f, 0.0f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}}},
    {{{-0.5f, 0.0f,  0.5f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}}},
};

static const unsigned int plane_indices[] = {
    0, 2, 1,
    2, 0, 3,
};

static const Vertex quad_vertices[] = {
    {{{-0.5f, -0.5f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{ 0.5f, -0.5f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{ 0.5f,  0.5f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}},
    {{{-0.5f,  0.5f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}},
};

static const unsigned int quad_indices[] = {
    0, 1, 2,
    2, 3, 0,
};

enum {
    UV_SPHERE_SEGMENT_COUNT = 16,
    UV_SPHERE_RING_COUNT = 8,
    UV_SPHERE_VERTEX_COUNT = 
        (UV_SPHERE_RING_COUNT + 1) *
        (UV_SPHERE_SEGMENT_COUNT + 1),
    UV_SPHERE_INDEX_COUNT =
        UV_SPHERE_RING_COUNT *
        UV_SPHERE_SEGMENT_COUNT *
        6,
};

static Vertex uv_sphere_vertices[UV_SPHERE_VERTEX_COUNT];
static unsigned int uv_sphere_indices[UV_SPHERE_INDEX_COUNT];
static bool uv_sphere_initialized = false;

enum {
    CYLINDER_SEGMENT_COUNT = 16,
    CYLINDER_SIDE_VERTEX_COUNT =
        (CYLINDER_SEGMENT_COUNT + 1) * 2,
    CYLINDER_CAP_VERTEX_COUNT =
        1 + (CYLINDER_SEGMENT_COUNT + 1),
    CYLINDER_VERTEX_COUNT =
        CYLINDER_SIDE_VERTEX_COUNT +
        (CYLINDER_CAP_VERTEX_COUNT * 2),
    CYLINDER_INDEX_COUNT =
        (CYLINDER_SEGMENT_COUNT * 6) +
        (CYLINDER_SEGMENT_COUNT * 3 * 2),
};

static Vertex cylinder_vertices[CYLINDER_VERTEX_COUNT];
static unsigned int cylinder_indices[CYLINDER_INDEX_COUNT];
static bool cylinder_initialized = false;

static void initialize_uv_sphere_data(void)
{
    if (uv_sphere_initialized)
    {
        return;
    }

    const float pi = 3.14159265f;
    const float radius = 0.5f;

    size_t vertex_index = 0;

    for (size_t ring = 0; ring <= UV_SPHERE_RING_COUNT; ring++)
    {
        float v = (float)ring / (float)UV_SPHERE_RING_COUNT;
        float theta = v * pi;

        float y = cosf(theta) * radius;

        for (size_t segment = 0;
             segment <= UV_SPHERE_SEGMENT_COUNT;
             segment++)
        {
            float u = (float)segment /
                (float)UV_SPHERE_SEGMENT_COUNT;
            float phi = u * 2.0f * pi;

            float normal_x = sinf(theta) * cosf(phi);
            float normal_y = cosf(theta);
            float normal_z = sinf(theta) * sinf(phi);

            uv_sphere_vertices[vertex_index] = (Vertex){
                .position = {{
                    normal_x * radius,
                    y,
                    normal_z * radius,
                }},
                .color = {{1.0f, 1.0f, 1.0f}},
                .uv = {{u, v}},
                .normal = {{
                    normal_x,
                    normal_y,
                    normal_z,
                }},
            };

            vertex_index++;
        }
    }

    size_t index = 0;

    for (size_t ring = 0; ring < UV_SPHERE_RING_COUNT; ring++)
    {
        for (size_t segment = 0;
             segment < UV_SPHERE_SEGMENT_COUNT;
             segment++)
        {
            unsigned int top_left =
                (unsigned int)(
                        ring * (UV_SPHERE_SEGMENT_COUNT + 1) +
                        segment
                    );
            unsigned int top_right = top_left + 1;
            unsigned int bottom_left = 
                (unsigned int)(
                        (ring + 1) *
                        (UV_SPHERE_SEGMENT_COUNT + 1) +
                        segment
                    );
            unsigned int bottom_right = bottom_left + 1;

            uv_sphere_indices[index++] = top_left;
            uv_sphere_indices[index++] = top_right;
            uv_sphere_indices[index++] = bottom_left;

            uv_sphere_indices[index++] = top_right;
            uv_sphere_indices[index++] = bottom_right;
            uv_sphere_indices[index++] = bottom_left;
        }
    }
    uv_sphere_initialized = true;
}

static void initialize_cylinder_data(void)
{
    if (cylinder_initialized)
    {
        return;
    }

    const float pi = 3.14159265f;
    const float radius = 0.5f;
    const float half_height = 0.5f;

    size_t vertex_index = 0;

    for (size_t segment = 0;
         segment <= CYLINDER_SEGMENT_COUNT;
         segment++)
    {
        float u = (float)segment /
            (float)CYLINDER_SEGMENT_COUNT;
        float phi = u * 2.0f * pi;

        float x = cosf(phi) * radius;
        float z = sinf(phi) * radius;

        cylinder_vertices[vertex_index++] = (Vertex){
            .position = {{x, -half_height, z}},
            .color = {{1.0f, 1.0f, 1.0f}},
            .uv = {{u, 0.0f}},
            .normal = {{x / radius, 0.0f, z / radius}},
        };

        cylinder_vertices[vertex_index++] = (Vertex){
            .position = {{x, half_height, z}},
            .color = {{1.0f, 1.0f, 1.0f}},
            .uv = {{u, 1.0f}},
            .normal = {{x / radius, 0.0f, z / radius}},
        };
    }

    size_t index = 0;

    for (size_t segment = 0;
         segment < CYLINDER_SEGMENT_COUNT;
         segment++)
    {
        unsigned int bottom_left = (unsigned int)(segment * 2);
        unsigned int top_left = bottom_left + 1;
        unsigned int bottom_right = bottom_left + 2;
        unsigned int top_right = bottom_left + 3;

        cylinder_indices[index++] = bottom_left;
        cylinder_indices[index++] = top_left;
        cylinder_indices[index++] = bottom_right;

        cylinder_indices[index++] = bottom_right;
        cylinder_indices[index++] = top_left;
        cylinder_indices[index++] = top_right;
    }

    unsigned int top_center = (unsigned int)vertex_index++;

    cylinder_vertices[top_center] = (Vertex){
        .position = {{0.0f, half_height, 0.0f}},
        .color = {{1.0f, 1.0f, 1.0f}},
        .uv = {{0.5f, 0.5f}},
        .normal = {{0.0f, 1.0f, 0.0f}},
    };
    
    unsigned int top_ring_start = (unsigned int)vertex_index;

    for(size_t segment = 0;
        segment <= CYLINDER_SEGMENT_COUNT;
        segment++)
    {
        float u = (float)segment /
            (float)CYLINDER_SEGMENT_COUNT;
        float phi = u * 2.0f * pi;

        float x = cosf(phi) * radius;
        float z = sinf(phi) * radius;

        cylinder_vertices[vertex_index++] = (Vertex){
            .position = {{x, half_height, z}},
            .color = {{1.0f, 1.0f, 1.0f}},
            .uv = {{
                0.5f + (x / radius) * 0.5f,
                0.5f + (z / radius) * 0.5f,
            }},
            .normal = {{0.0f, 1.0f, 0.0f}},
        };
    }

    unsigned int bottom_center = (unsigned int)vertex_index++;

    cylinder_vertices[bottom_center] = (Vertex){
        .position = {{0.0f, -half_height, 0.0f}},
        .color = {{1.0f, 1.0f, 1.0f}},
        .uv = {{0.5f, 0.5f}},
        .normal = {{0.0f, -1.0f, 0.0f}},
    };

    unsigned int bottom_ring_start = (unsigned int)vertex_index;

    for (size_t segment = 0;
         segment <= CYLINDER_SEGMENT_COUNT;
         segment++)
    {
        float u = (float)segment /
            (float)CYLINDER_SEGMENT_COUNT;
        float phi = u * 2.0f * pi;

        float x = cosf(phi) * radius;
        float z = sinf(phi) * radius;
        
        cylinder_vertices[vertex_index++] = (Vertex){
            .position = {{x, -half_height, z}},
            .color = {{1.0f, 1.0f, 1.0f}},
            .uv = {{
                0.5f + (x / radius) * 0.5f,
                0.5f + (z / radius) * 0.5f,
            }},
            .normal = {{0.0f, -1.0f, 0.0f}},
        };
    }

    for (size_t segment = 0;
         segment < CYLINDER_SEGMENT_COUNT;
         segment++)
    {
        unsigned int top_current =  
            top_ring_start + (unsigned int)segment;
        unsigned int top_next = top_current + 1;

        cylinder_indices[index++] = top_center;
        cylinder_indices[index++] = top_next;
        cylinder_indices[index++] = top_current;

        unsigned int bottom_current =
            bottom_ring_start + (unsigned int)segment;
        unsigned int bottom_next = bottom_current + 1;

        cylinder_indices[index++] = bottom_center;
        cylinder_indices[index++] = bottom_current;
        cylinder_indices[index++] = bottom_next;
    }

    cylinder_initialized = true;
}

static void init_primitive_mesh_data(PrimitiveMeshData *primitive_mesh)
{
    primitive_mesh->index_count = 0;
    primitive_mesh->vertex_count = 0;
    primitive_mesh->indices = 0;
    primitive_mesh->vertices = 0;
}
static void primitive_mesh_data_assign(
    PrimitiveMeshData *out_data, 
    const Vertex *vertices, 
    size_t vertex_count,
    const unsigned int *indices,
    size_t index_count
)
{
    out_data->vertices = vertices;
    out_data->vertex_count = vertex_count;
    out_data->indices = indices;
    out_data->index_count = index_count;
}

bool primitive_mesh_get_data(BuiltinPrimitiveType type, PrimitiveMeshData *out_data)
{
    if (out_data == NULL)
    {
        return false;
    }

    init_primitive_mesh_data(out_data);

    switch (type)
    {
        case BUILTIN_PRIMITIVE_CUBE:
            primitive_mesh_data_assign(
                out_data, 
                cube_vertices, 
                sizeof(cube_vertices) / sizeof(cube_vertices[0]),
                cube_indices,
                sizeof(cube_indices) / sizeof(cube_indices[0])
            );
            return true;
        case BUILTIN_PRIMITIVE_PLANE:
            primitive_mesh_data_assign(
                out_data,
                plane_vertices, 
                sizeof(plane_vertices) / sizeof(plane_vertices[0]),
                plane_indices,
                sizeof(plane_indices) / sizeof(plane_indices[0])
            );
            return true;
        case BUILTIN_PRIMITIVE_QUAD:
            primitive_mesh_data_assign(
                out_data,
                quad_vertices,
                sizeof(quad_vertices) / sizeof(quad_vertices[0]),
                quad_indices,
                sizeof(quad_indices) / sizeof(quad_indices[0])
            );
            return true;
        case BUILTIN_PRIMITIVE_UV_SPHERE:
            initialize_uv_sphere_data();

            primitive_mesh_data_assign(
                out_data, 
                uv_sphere_vertices, 
                UV_SPHERE_VERTEX_COUNT, 
                uv_sphere_indices, 
                UV_SPHERE_INDEX_COUNT
            );

            return true;
        case BUILTIN_PRIMITIVE_CYLINDER:
            initialize_cylinder_data();

            primitive_mesh_data_assign(
                out_data, 
                cylinder_vertices, 
                CYLINDER_VERTEX_COUNT, 
                cylinder_indices, 
                CYLINDER_INDEX_COUNT
            );

            return true;
        default:
            return false;
    }
}


