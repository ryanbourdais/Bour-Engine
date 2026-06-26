#include "model.h"
#include <cgltf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void model_init(Model *model)
{
    model->meshes = malloc(4 * sizeof(Mesh));

    if(model->meshes == NULL)
    {
        fprintf(stderr, "Failed to allocate model meshes\n");
        model->count = 0;
        model->capacity = 0;
        return;
    }

    model->count = 0;
    model->capacity = 4;
}

static bool model_add_mesh(Model *model, Mesh mesh)
{
    if (model->count == model->capacity)
    {
        size_t new_capacity = model->capacity * 2;
        Mesh *new_items = realloc(model->meshes, new_capacity * sizeof(Mesh));

        if (new_items == NULL)
        {
            fprintf(stderr, "Failed to grow model mesh array\n");
            return false;
        }

        model->meshes = new_items;
        model->capacity = new_capacity;
    }

    model->meshes[model->count] = mesh;
    model->count++;

    return true;
}

void model_free(Model *model)
{
    if (model == NULL)
    {
        return;
    }

    for (size_t i = 0; i < model->count; i++)
    {
        Mesh *mesh = &model->meshes[i];

        glDeleteBuffers(1, &mesh->position_vbo);
        glDeleteBuffers(1, &mesh->color_vbo);
        glDeleteBuffers(1, &mesh->uv_vbo);
        glDeleteBuffers(1, &mesh->normal_vbo);
        glDeleteBuffers(1, &mesh->ebo);
        glDeleteVertexArrays(1, &mesh->vao);

        if (mesh->texture)
        {
            glDeleteTextures(1, &mesh->texture);
        }

        if (mesh->texture2)
        {
            glDeleteTextures(1, &mesh->texture2);
        }
    }

    free(model->meshes);
    model->meshes = NULL;
    model->count = 0;
    model->capacity = 0;
}

static void inspect_primitive(cgltf_primitive *primitive)
{
    cgltf_accessor *position_accessor = NULL;
    cgltf_accessor *normal_accessor = NULL;
    cgltf_accessor *texcoord_accessor = NULL;

    for (cgltf_size i = 0; i < primitive->attributes_count; i++)
    {
        cgltf_attribute *attribute = &primitive->attributes[i];

        if (attribute->type == cgltf_attribute_type_position)
        {
            position_accessor = attribute->data;
        }
        else if (attribute->type == cgltf_attribute_type_normal)
        {
            normal_accessor = attribute->data;
        }
        else if (
            attribute->type == cgltf_attribute_type_texcoord &&
            attribute->index == 0
        )
        {
            texcoord_accessor = attribute->data;
        }
    }

    printf("    Primitive type: %d\n", primitive->type);

    if (position_accessor)
    {
        printf("    Positions: %zu\n", position_accessor->count);
    }
    else {
        printf("    Positions: missing\n");
    }

    if (normal_accessor)
    {
        printf("    Normals: %zu\n", normal_accessor->count);
    }
    else {
        printf("    Normals: missing\n");
    }

    if (texcoord_accessor)
    {
        printf("    Texcoords: %zu\n", texcoord_accessor->count);
    }
    else {
        printf("    Texcoords: missing\n");
    }

    if (primitive->indices)
    {
        printf("    Indices: %zu\n", primitive->indices->count);
    }
    else {
        printf("    Indices: missing\n");
    }
}

static int create_mesh_from_primitive(Model *model, cgltf_primitive *primitive)
{
    if (primitive->type != cgltf_primitive_type_triangles)
    {
        return 0;
    }

    cgltf_accessor *position_accessor = NULL;
    cgltf_accessor *normal_accessor = NULL;
    cgltf_accessor *texcoord_accessor = NULL;

    for(cgltf_size i = 0; i < primitive->attributes_count; i++)
    {
        cgltf_attribute *attribute = &primitive->attributes[i];

        if(attribute->type == cgltf_attribute_type_position)
        {
            position_accessor = attribute->data;
        }
        else if(attribute->type == cgltf_attribute_type_normal)
        {
            normal_accessor = attribute->data;
        }
        else if(
            attribute->type == cgltf_attribute_type_texcoord &&
            attribute->index == 0
        )
        {
            texcoord_accessor = attribute->data;
        }
    }

    if(position_accessor == NULL)
    {
        fprintf(stderr, "glTF primitive missing POSITION attribute\n");
        return 1;
    }

    size_t vertex_count = position_accessor->count;
    Vertex *vertices = malloc(vertex_count * sizeof(Vertex));

    if (vertices == NULL)
    {
        fprintf(stderr, "Failed to allocate model vertices\n");
        return 1;
    }

    for (size_t i = 0; i < vertex_count; i++)
    {
        Vertex vertex = {0};

        float position[3] = {0.0f, 0.0f, 0.0f};
        cgltf_accessor_read_float(position_accessor, i, position, 3);

        vertex.position = (vec3s){{position[0], position[1], position[2]}};
        vertex.color = (vec3s){{1.0f, 1.0f, 1.0f}};

        if (normal_accessor)
        {
            float normal[3] = {0.0f, 0.0f, 1.0f};
            cgltf_accessor_read_float(normal_accessor, i, normal, 3);
            vertex.normal = (vec3s){{normal[0], normal[1], normal[2]}};
        }
        else {
            vertex.normal = (vec3s){{0.0f, 0.0f, 1.0f}};
        }

        if(texcoord_accessor)
        {
            float uv[2] = {0.0f, 0.0f};
            cgltf_accessor_read_float(texcoord_accessor, i, uv, 2);
            vertex.uv = (vec2s){{uv[0], uv[1]}};
        }
        else {
            vertex.uv = (vec2s){{0.0f, 0.0f}};
        }

        vertices[i] = vertex;
    }

    size_t index_count = 0;
    unsigned int *indices = NULL;

    if(primitive->indices)
    {
        index_count = primitive->indices->count;
        indices = malloc(index_count * sizeof(unsigned int));

        if (indices == NULL)
        {
            free(vertices);
            fprintf(stderr, "Failed to allocate model indices\n");
            return 1;
        }

        for (size_t i = 0; i < index_count; i++)
        {
            indices[i] = (unsigned int)cgltf_accessor_read_index(primitive->indices, i);
        }
    }

    else
    {
        index_count = vertex_count;
        indices = malloc(index_count * sizeof(unsigned int));

        if (indices == NULL)
        {
            free(vertices);
            fprintf(stderr, "Failed to allocate generated model indices\n");
            return 1;
        }

        for (size_t i = 0; i < index_count; i++)
        {
            indices[i] = (unsigned int)i;
        }
    }

    Mesh mesh = {0};

    int status = create_mesh_from_vertices(
        &mesh,
        vertices,
        vertex_count,
        indices,
        (GLsizei)index_count
    );

    free(vertices);
    free(indices);

    if (status != 0)
    {
        return 1;
    }

    if (!model_add_mesh(model, mesh))
    {
        return 1;
    }
    return 0;
}

int model_load_gltf(Model *model, const char *path)
{
    cgltf_options options = {0};
    cgltf_data *data = NULL;

    cgltf_result result = cgltf_parse_file(&options, path, &data);
    if (result != cgltf_result_success){ return 1; }

    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) 
    { 
        cgltf_free(data);
        return 1; 
    }

    model_init(model);

    printf("Loaded glTF file: %s\n", path);
    printf("Meshes: %zu\n", data->meshes_count);

    for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; mesh_index++)
    {
        cgltf_mesh *gltf_mesh = &data->meshes[mesh_index];

        printf("Mesh %zu\n", mesh_index);
        printf("Primitives: %zu\n", gltf_mesh->primitives_count);

        for (cgltf_size primitive_index = 0; primitive_index < gltf_mesh->primitives_count; primitive_index++)
        {
            cgltf_primitive *primitive = &gltf_mesh->primitives[primitive_index];

            printf("    Primitive %zu\n", primitive_index);
            inspect_primitive(primitive);

            if(create_mesh_from_primitive(model, primitive) != 0)
            {
                cgltf_free(data);
                model_free(model);
                return 1;
            }
        }
        printf("Engine meshes created: %zu\n", model->count);
    }

    cgltf_free(data);

    return 0;
}

void draw_model(Model *model, GLint model_location, mat4 model_matrix)
{
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (float *)model_matrix);

    for (size_t i = 0; i < model->count; i++)
    {
        Mesh *mesh = &model->meshes[i];

        glBindVertexArray(mesh->vao);

        glDrawElements(
            GL_TRIANGLES,
            mesh->index_count,
            GL_UNSIGNED_INT,
            0
        );
    }
}