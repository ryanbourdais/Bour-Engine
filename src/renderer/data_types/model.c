#include "model.h"
#include <cgltf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "texture.h"

typedef struct ModelImportDiagnostics
{
    size_t nodes_seen;
    size_t mesh_nodes_seen;

    size_t primitives_seen;
    size_t primitives_imported;
    size_t primitives_skipped_non_triangles;

    size_t missing_positions;
    size_t missing_normals;
    size_t missing_texcoords;
    size_t missing_indices;

    size_t materials_seen;
    size_t materials_missing;

    size_t diffuse_textures_found;
    size_t diffuse_textures_missing;

    size_t alpha_opaque;
    size_t alpha_mask;
    size_t alpha_blend;

    size_t fallback_diffuse_used;

} ModelImportDiagnostics;

typedef struct TransparentModelMesh {
    ModelMesh *model_mesh;
    float distance;
} TransparentModelMesh;

static void print_import_diagnostics(const Model *model, const ModelImportDiagnostics *diagnostics)
{
    printf("\nModel import summary\n");
    printf("  Nodes seen: %zu\n", diagnostics->nodes_seen);
    printf("  Mesh nodes seen: %zu\n", diagnostics->mesh_nodes_seen);

    printf("  Primitives seen: %zu\n", diagnostics->primitives_seen);
    printf("  Primitives imported: %zu\n", diagnostics->primitives_imported);
    printf("  Primitives skipped non-triangles: %zu\n", diagnostics->primitives_skipped_non_triangles);

    printf("  Missing POSITION: %zu\n", diagnostics->missing_positions);
    printf("  Missing NORMAL: %zu\n", diagnostics->missing_normals);
    printf("  Missing TEXCOORD_0: %zu\n", diagnostics->missing_texcoords);
    printf("  Missing indices: %zu\n", diagnostics->missing_indices);

    printf("  Materials seen: %zu\n", diagnostics->materials_seen);
    printf("  Materials missing: %zu\n", diagnostics->materials_missing);

    printf("  Diffuse textures found: %zu\n", diagnostics->diffuse_textures_found);
    printf("  Diffuse textures missing: %zu\n", diagnostics->diffuse_textures_missing);
    printf("  Fallback diffuse used: %zu\n", diagnostics->fallback_diffuse_used);

    printf("  Alpha opaque: %zu\n", diagnostics->alpha_opaque);
    printf("  Alpha mask: %zu\n", diagnostics->alpha_mask);
    printf("  Alpha blend: %zu\n", diagnostics->alpha_blend);

    printf("  Engine meshes created: %zu\n", model->count);
    printf("  Unique textures loaded: %zu\n", model->texture_cache_count);
}

static bool model_add_mesh(Model *model, Mesh mesh, Material material, mat4 transform)
{
    if (model->count == model->capacity)
    {
        size_t new_capacity = model->capacity * 2;
        ModelMesh *new_items = realloc(model->meshes, new_capacity * sizeof(ModelMesh));

        if (new_items == NULL)
        {
            fprintf(stderr, "Failed to grow model mesh array\n");
            return false;
        }

        model->meshes = new_items;
        model->capacity = new_capacity;
    }

    model->meshes[model->count].mesh = mesh;
    model->meshes[model->count].material = material;
    glm_mat4_copy(transform, model->meshes[model->count].transform);
    model->count++;

    return true;
}

static float model_mesh_distance_to_camera(ModelMesh *model_mesh, mat4 model_matrix, vec3 camera_position)
{
    mat4 final_model;
    glm_mat4_mul(model_matrix, model_mesh->transform, final_model);

    vec3 mesh_position = {
        final_model[3][0],
        final_model[3][1],
        final_model[3][2]
    };

    vec3 difference;
    glm_vec3_sub(camera_position, mesh_position, difference);

    return glm_vec3_dot(difference, difference);
}

static int compare_transparent_model_meshes(const void *a, const void *b)
{
    const TransparentModelMesh *mesh_a = a;
    const TransparentModelMesh *mesh_b = b;

    if (mesh_a->distance < mesh_b->distance)
    {
        return 1;
    }

    if (mesh_a->distance > mesh_b->distance)
    {
        return -1;
    }

    return 0;
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

static int create_mesh_from_primitive(Model *model, cgltf_primitive *primitive, mat4 transform, const char *model_directory, ModelImportDiagnostics *diagnostics)
{

    diagnostics->primitives_seen++;

    if (primitive->type != cgltf_primitive_type_triangles)
    {
        diagnostics->primitives_skipped_non_triangles++;
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

    
    if (normal_accessor == NULL)
    {
        diagnostics->missing_normals++;
    }
    if (texcoord_accessor == NULL)
    {
        diagnostics->missing_texcoords++;
    }
    if (primitive->indices == NULL)
    {
        diagnostics->missing_indices++;
    }

    if(position_accessor == NULL)
    {
        diagnostics->missing_positions++;
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

    Material engine_material = {0};

    engine_material.diffuse_texture = 0;
    engine_material.specular_texture = 0;
    engine_material.diffuse_color = (vec4s){{1.0f, 1.0f, 1.0f, 1.0f}};
    engine_material.shininess = 32.0f;
    engine_material.alpha_mode = ALPHA_MODE_OPAQUE;
    engine_material.alpha_cutoff = 0.5f;
    engine_material.double_sided = false;

    int status = create_mesh_from_vertices(
        &mesh,
        vertices,
        vertex_count,
        indices,
        (GLsizei)index_count
    );

    
    cgltf_material *gltf_material = primitive->material;

    if (gltf_material)
    {
        diagnostics->materials_seen++;

        engine_material.double_sided = gltf_material->double_sided;

        if (gltf_material->alpha_mode == cgltf_alpha_mode_mask)
        {
            diagnostics->alpha_mask++;
            engine_material.alpha_mode = ALPHA_MODE_MASK;
            engine_material.alpha_cutoff = gltf_material->alpha_cutoff;
        }
        else if (gltf_material->alpha_mode == cgltf_alpha_mode_blend)
        {
            diagnostics->alpha_blend++;
            engine_material.alpha_mode = ALPHA_MODE_BLEND;
            engine_material.alpha_cutoff = 0.0f;
        }
        else {
            diagnostics->alpha_opaque++;
            engine_material.alpha_mode = ALPHA_MODE_OPAQUE;
            engine_material.alpha_cutoff = 0.5f;
        }

        if (gltf_material->has_pbr_specular_glossiness)
        {
            engine_material.diffuse_color = (vec4s){{
                gltf_material->pbr_specular_glossiness.diffuse_factor[0],
                gltf_material->pbr_specular_glossiness.diffuse_factor[1],
                gltf_material->pbr_specular_glossiness.diffuse_factor[2],
                gltf_material->pbr_specular_glossiness.diffuse_factor[3],
            }};
        }
        else {
            diagnostics->materials_missing++;
            engine_material.diffuse_color = (vec4s){{
                gltf_material->pbr_metallic_roughness.base_color_factor[0],
                gltf_material->pbr_metallic_roughness.base_color_factor[1],
                gltf_material->pbr_metallic_roughness.base_color_factor[2],
                gltf_material->pbr_metallic_roughness.base_color_factor[3]
            }};
        }
        cgltf_texture *diffuse_texture = NULL;

        if (gltf_material->pbr_metallic_roughness.base_color_texture.texture)
        {
            diffuse_texture = gltf_material->pbr_metallic_roughness.base_color_texture.texture;
        }
        else if (
            gltf_material->has_pbr_specular_glossiness &&
            gltf_material->pbr_specular_glossiness.diffuse_texture.texture
        )
        {
            diffuse_texture = gltf_material->pbr_specular_glossiness.diffuse_texture.texture;
        }

        if (diffuse_texture && diffuse_texture->image && diffuse_texture->image->uri)
        {
            diagnostics->diffuse_textures_found++;

            char texture_path[512] = {0};

            snprintf(
                texture_path,
                sizeof(texture_path),
                "%s/%s",
                model_directory,
                diffuse_texture->image->uri
            );

            if (model_load_cached_texture(model, texture_path, &engine_material.diffuse_texture) != 0)
            {
                fprintf(stderr, "Failed to load base color texture %s\n", texture_path);
            }
        } else {
            diagnostics->diffuse_textures_missing++;
        }
    }

    if (engine_material.diffuse_texture == 0)
    {
        diagnostics->fallback_diffuse_used++;
        if (model_get_fallback_white_texture(model, &engine_material.diffuse_texture) != 0)
        {
            fprintf(stderr, "Failed to get fallback diffuse texture\n");
        }
    }

    free(vertices);
    free(indices);

    if (status != 0)
    {
        return 1;
    }

    if (!model_add_mesh(model, mesh, engine_material, transform))
    {
        return 1;
    }

    diagnostics->primitives_imported++;

    return 0;
}

static int process_node(Model *model, cgltf_node *node, const char *model_directory, ModelImportDiagnostics *diagnostics)
{
    mat4 transform;
    cgltf_node_transform_world(node, (float *)transform);

    diagnostics->nodes_seen++;

    if (node->mesh)
    {
        diagnostics->mesh_nodes_seen++;
    }

    if(node->mesh)
    {
        cgltf_mesh *gltf_mesh = node->mesh;

        for (cgltf_size primitive_index = 0; primitive_index < gltf_mesh->primitives_count; primitive_index++)
        {
            cgltf_primitive *primitive = &gltf_mesh->primitives[primitive_index];

            if(create_mesh_from_primitive(model, primitive, transform, model_directory, diagnostics) != 0) { return 1;}
        }
    }

    for (cgltf_size child_index = 0; child_index < node->children_count; child_index++)
    {
            if (process_node(model, node->children[child_index], model_directory, diagnostics) != 0) { return 1;}
    }
    return 0;
}

static bool model_add_cached_texture(Model *model, const char *path, GLuint texture)
{
    if (model->texture_cache_count == model->texture_cache_capacity)
    {
        size_t  new_capacity = model->texture_cache_capacity * 2;

        TextureCacheEntry *new_items = realloc(
            model->texture_cache,
            new_capacity * sizeof(TextureCacheEntry)
        );

        if (new_items == NULL)
        {
            fprintf(stderr, "Failed to grow model texture cache\n");
            return false;
        }

        model->texture_cache = new_items;
        model->texture_cache_capacity = new_capacity;
    }

    TextureCacheEntry *entry = &model->texture_cache[model->texture_cache_count];

    snprintf(entry->path, sizeof(entry->path), "%s", path);
    entry->texture = texture;

    model->texture_cache_count++;

    return true;
}

static bool model_get_cached_texture(Model *model, const char *path, GLuint *out_texture)
{
    for (size_t i = 0; i < model->texture_cache_count; i++)
    {
        TextureCacheEntry *entry = &model->texture_cache[i];

        if (strcmp(entry->path, path) == 0)
        {
            *out_texture = entry->texture;
            return true;
        }
    }

    return false;
}

static int model_load_cached_texture(Model *model, const char *path, GLuint *out_texture)
{
    if (model_get_cached_texture(model, path, out_texture))
    {
        return 0;
    }

    GLuint texture = 0;

    if (load_texture(&texture, path) != 0)
    {
        return 1;
    }

    if (!model_add_cached_texture(model, path, texture))
    {
        glDeleteTextures(1, &texture);
        return 1;
    }

    *out_texture = texture;
    
    return 0;
}

static int model_get_fallback_white_texture(Model *model, GLuint *out_texture)
{
    const char *fallback_key = "__fallback_white__";

    if (model_get_cached_texture(model, fallback_key, out_texture))
    {
        return 0;
    }

    GLuint texture = 0;

    if (create_solid_color_texture(&texture, 255, 255, 255, 255) != 0)
    {
        return 1;
    }

    if (!model_add_cached_texture(model, fallback_key, texture))
    {
        glDeleteTextures(1, &texture);
        return 1;
    }

    *out_texture = texture;

    return 0;
}

static void draw_model_mesh(ModelMesh *model_mesh, GLint model_location, MaterialUniforms *material_uniforms, mat4 model_matrix)
{
    mat4 final_model;
    glm_mat4_mul(model_matrix, model_mesh->transform, final_model);

    glUniformMatrix4fv(model_location, 1, GL_FALSE, (float *)final_model);

    Mesh *mesh = &model_mesh->mesh;
    Material *material = &model_mesh->material;

    if (material->double_sided)
    {
        glDisable(GL_CULL_FACE);
    }
    else{
        glEnable(GL_CULL_FACE);
    }

    upload_material_diffuse_color(material_uniforms, material->diffuse_color);
    upload_material_shininess(material_uniforms, material->shininess);
    upload_material_alpha(material_uniforms, material->alpha_mode, material->alpha_cutoff);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material->diffuse_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material->specular_texture);

    glBindVertexArray(mesh->vao);

    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
}

static void draw_model_mesh_instanced(ModelMesh *model_mesh, GLint model_location, MaterialUniforms *material_uniforms, mat4 model_matrix, GLuint instance_vbo, GLsizei instance_count)
{
    Mesh *mesh = &model_mesh->mesh;
    Material *material = &model_mesh->material;

    if (material->alpha_mode == ALPHA_MODE_BLEND)
    {
        return;
    }

    if (material->double_sided)
    {
        glDisable(GL_CULL_FACE);
    }
    else {
        glEnable(GL_CULL_FACE);
    }

    upload_material_diffuse_color(material_uniforms, material->diffuse_color);
    upload_material_shininess(material_uniforms, material->shininess);
    upload_material_alpha(material_uniforms, material->alpha_mode, material->alpha_cutoff);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material->diffuse_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material->specular_texture);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

    size_t vec4_size = sizeof(float) * 4;

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *)(0 * vec4_size));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *)(1 * vec4_size));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *)(2 * vec4_size));
    glVertexAttribDivisor(6, 1);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *)(3 * vec4_size));
    glVertexAttribDivisor(7, 1);

    mat4 mesh_model;
    glm_mat4_mul(model_matrix, model_mesh->transform, mesh_model);

    glUniformMatrix4fv(model_location, 1, GL_FALSE, (float *)mesh_model);

    glDrawElementsInstanced(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0, instance_count);
}

void draw_model_instanced(Model *model, GLint model_location, MaterialUniforms *material_uniforms, mat4 model_matrix, GLuint instance_vbo, GLsizei instance_count)
{
    for(size_t i = 0; i < model->count; i++)
    {
        draw_model_mesh_instanced(&model->meshes[i], model_location, material_uniforms, model_matrix, instance_vbo, instance_count);
    }
}

void model_init(Model *model)
{
    model->meshes = malloc(4 * sizeof(ModelMesh));

    if(model->meshes == NULL)
    {
        fprintf(stderr, "Failed to allocate model meshes\n");
        model->count = 0;
        model->capacity = 0;
        return;
    }

    model->count = 0;
    model->capacity = 4;

    model->texture_cache = malloc(8 * sizeof(TextureCacheEntry));

    if(model->texture_cache == NULL)
    {
        fprintf(stderr, "Failed to allocate model texture cache\n");
        free(model->meshes);
        model->meshes = NULL;
        model->count = 0;
        model->capacity = 0;
        model->texture_cache_count = 0;
        model->texture_cache_capacity = 0;
        return;
    }

    model->texture_cache_count = 0;
    model->texture_cache_capacity = 8;

}

void model_free(Model *model)
{
    if (model == NULL)
    {
        return;
    }

    for (size_t i = 0; i < model->count; i++)
    {
        Mesh *mesh = &model->meshes[i].mesh;

        glDeleteBuffers(1, &mesh->vertex_vbo);
        glDeleteBuffers(1, &mesh->ebo);
        glDeleteVertexArrays(1, &mesh->vao);
    }

    for (size_t i = 0; i < model->texture_cache_count; i++)
    {
        if (model->texture_cache[i].texture)
        {
            glDeleteTextures(1, &model->texture_cache[i].texture);
        }
    }

    free(model->texture_cache);
    model->texture_cache = NULL;
    model->texture_cache_count = 0;
    model->texture_cache_capacity = 0;

    free(model->meshes);
    model->meshes = NULL;
    model->count = 0;
    model->capacity = 0;
}

int model_load_gltf(Model *model, const char *path)
{
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    ModelImportDiagnostics diagnostics = {0};

    char model_directory[512] = {0};

    const char *last_forward_slash = strrchr(path, '/');
    const char *last_back_slash = strrchr(path, '\\');

    const char *last_slash = last_forward_slash;

    if (last_back_slash && (!last_forward_slash || last_back_slash > last_forward_slash))
    {
        last_slash = last_back_slash;
    }

    if (last_slash)
    {
        size_t directory_length = (size_t)(last_slash - path);
        snprintf(model_directory, sizeof(model_directory), "%.*s", (int)directory_length, path);
    }
    else {
        snprintf(model_directory, sizeof(model_directory), ".");
    }


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

    cgltf_scene *scene = data->scene;

    if (scene == NULL && data->scenes_count > 0)
    {
        scene = &data->scenes[0];
    }
    if (scene == NULL)
    {
        fprintf(stderr, "glTF file has no scene\n");
        cgltf_free(data);
        model_free(model);
        return 1;
    }

    for (cgltf_size node_index = 0; node_index < scene->nodes_count; node_index++)
    {
        if (process_node(model, scene->nodes[node_index], model_directory, &diagnostics) != 0)
        {
            cgltf_free(data);
            model_free(model);
            return 1;
        }
    }

    print_import_diagnostics(model, &diagnostics);
    cgltf_free(data);

    return 0;
}

void draw_model(Model *model, GLint model_location, MaterialUniforms *material_uniforms, mat4 model_matrix, vec3 camera_position)
{

    TransparentModelMesh *transparent_meshes = malloc(model->count * sizeof(TransparentModelMesh));

    if (transparent_meshes == NULL)
    {
        fprintf(stderr, "Failed to allocate transparent mesh sort list\n");
        return;
    }

    size_t transparent_count = 0;

    glDepthMask(GL_TRUE);

    for (size_t i = 0; i < model->count; i++)
    {
        ModelMesh *model_mesh = &model->meshes[i];

        if (model_mesh->material.alpha_mode == ALPHA_MODE_BLEND)
        {
            transparent_meshes[transparent_count].model_mesh = model_mesh;
            transparent_meshes[transparent_count].distance = model_mesh_distance_to_camera(model_mesh, model_matrix, camera_position);
            transparent_count++;
            continue;
        }

        draw_model_mesh(model_mesh, model_location, material_uniforms, model_matrix);
    }

    qsort(transparent_meshes, transparent_count, sizeof(TransparentModelMesh), compare_transparent_model_meshes);

    glDepthMask(GL_FALSE);

    for (size_t i = 0; i < transparent_count; i++)
    {
        draw_model_mesh(transparent_meshes[i].model_mesh, model_location, material_uniforms, model_matrix);
    }

    glDepthMask(GL_TRUE);

    free(transparent_meshes);
}
