#include "renderer.h"
#include <GL/gl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cglm/struct.h>
#include <cglm/mat4.h>

#include "data_types/mesh.h"
#include "data_types/texture.h"
#include "data_types/lightObject.h"
#include "data_types/material.h"
#include "data_types/model.h"
#include "data_types/skybox.h"
#include "data_types/renderTarget.h"
#include "data_types/primitive_mesh_resources.h"
#include "data_types/programmable_mesh_resources.h"

#include "renderer_data.h"
#include "shaders.h"

struct RendererState
{
    mat4 projection;
    Model test_model;
    PrimitiveMeshResources primitive_meshes;
    ProgrammableMeshResources programmable_meshes;
    Material primitive_material;
    Skybox skybox;

    RenderTarget scene_target;
    MsaaRenderTarget scene_msaa_target;

    DirectionalLightUniforms directional_light_uniforms;
    PointLightUniforms point_light_uniforms[MAX_SHADER_POINT_LIGHTS];
    SpotLightUniforms spot_light_uniforms[MAX_SHADER_SPOT_LIGHTS];
    MaterialUniforms material_uniforms;

    GLuint camera_ubo;
    GLuint screen_shader_program;
    GLuint screen_quad_vao;
    GLuint screen_quad_vbo;
    GLint screen_texture_location;

    GLuint shader_program;
    GLint point_light_count_location;
    GLint spot_light_count_location;
    GLint model_location;

    const char *loaded_model_path;
};

static void draw_screen_quad(struct RendererState *renderer)
{
    glUseProgram(renderer->screen_shader_program);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->scene_target.color_texture);

    glBindVertexArray(renderer->screen_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

RendererStats renderer_get_stats(const Renderer *renderer)
{
    RendererStats stats = {0};

    if (renderer == NULL)
    {
        return stats;
    }

    stats.mesh_count = renderer->test_model.count;
    stats.texture_count = renderer->test_model.texture_cache_count;

    if (renderer->primitive_material.diffuse_texture != 0)
    {
        stats.texture_count++;
    }

    for (size_t i = 0; i < renderer->test_model.count; i++)
    {
        const ModelMesh *mesh = &renderer->test_model.meshes[i];

        stats.vertex_count += mesh->mesh.vertex_count;
        stats.triangle_count += mesh->mesh.index_count / 3;
    }

    for (size_t i = 0; i < BUILTIN_PRIMITIVE_COUNT; i++)
    {
        if (!renderer->primitive_meshes.loaded[i])
        {
            continue;
        }

        const Mesh *mesh = &renderer->primitive_meshes.meshes[i];

        stats.mesh_count++;
        stats.vertex_count += mesh->vertex_count;
        stats.triangle_count += mesh->index_count / 3;
    }

    for (size_t i = 0; i < renderer->programmable_meshes.resource_count; i++)
    {
        const ProgrammableMeshResource *resource =
            &renderer->programmable_meshes.resources[i];

        if (!resource->in_use)
        {
            continue;
        }

        const Mesh *mesh = &resource->mesh;

        stats.mesh_count++;
        stats.vertex_count += mesh->vertex_count;
        stats.triangle_count += mesh->index_count / 3;
    }

    return stats;
}

static bool renderer_get_model_stats(const Renderer *renderer, const char *model_path, RendererStats *out_stats)
{
    if (renderer == NULL || out_stats == NULL)
    {
        return false;
    }

    if (renderer->loaded_model_path == NULL || model_path == NULL || strcmp(renderer->loaded_model_path, model_path) != 0)
    {
        return false;
    }

    *out_stats = (RendererStats){0};

    out_stats->mesh_count = renderer->test_model.count;
    out_stats->texture_count = renderer->test_model.texture_cache_count;

    for (size_t i = 0; i < renderer->test_model.count; i++)
    {
        const ModelMesh *mesh = &renderer->test_model.meshes[i];

        out_stats->vertex_count += mesh->mesh.vertex_count;
        out_stats->triangle_count += mesh->mesh.index_count / 3;
    }

    return true;
}



RendererStats renderer_get_frame_stats(const Renderer *renderer, const RendererFrame *frame)
{
    RendererStats stats = renderer_get_stats(renderer);

    if (renderer == NULL || frame == NULL)
    {
        return stats;
    }

    for (size_t i = 0; i < frame->renderable_count; i++)
    {
        const RenderableDrawData *renderable = &frame->renderables[i];

        if (renderable->geometry_type == RENDERABLE_GEOMETRY_MODEL)
        {
            RendererStats model_stats = {0};
            if (!renderer_get_model_stats(renderer, renderable->model_path, &model_stats))
            {
                stats.missing_model_count++;
                continue;
            }

            stats.submitted_draw_count++;
            stats.submitted_mesh_count += model_stats.mesh_count;
            stats.submitted_vertex_count += model_stats.vertex_count;
            stats.submitted_triangle_count += model_stats.triangle_count;
        }
        else if (renderable->geometry_type == RENDERABLE_GEOMETRY_PRIMITIVE)
        {
            const Mesh *mesh = primitive_mesh_resources_get(
                &renderer->primitive_meshes, 
                renderable->primitive_type
            );
            
            if (mesh == NULL)
            {
                continue;
            }

            stats.submitted_draw_count++;
            stats.submitted_mesh_count++;
            stats.submitted_vertex_count += mesh->vertex_count;
            stats.submitted_triangle_count += mesh->index_count / 3;
        }
        else if (renderable->geometry_type == RENDERABLE_GEOMETRY_PROGRAMMABLE)
        {
            const ProgrammableMesh *mesh =
                renderable->programmable_mesh;

            if (mesh == NULL ||
                mesh->vertices == NULL ||
                mesh->indices == NULL ||
                mesh->vertex_count == 0 ||
                mesh->index_count == 0 )
            {
                continue;
            }

            stats.submitted_draw_count++;
            stats.submitted_mesh_count++;
            stats.submitted_vertex_count += mesh->vertex_count;
            stats.submitted_triangle_count += mesh->index_count / 3;
        }
    }
    return stats;
}

static void draw_primitive_mesh(
        struct RendererState *renderer,
        const Mesh *mesh,
        mat4s model_matrix)
{
    if (renderer == NULL || mesh == NULL)
    {
        return;
    }

    Material *material = &renderer->primitive_material;

    if (material->double_sided)
    {
        glDisable(GL_CULL_FACE);
    }
    else {
        glEnable(GL_CULL_FACE);
    }

    glUniformMatrix4fv(
        renderer->model_location,
        1,
        GL_FALSE,
        (float *)model_matrix.raw
    );

    upload_material_diffuse_color(
        &renderer->material_uniforms, 
        material->diffuse_color
    );

    upload_material_shininess(
        &renderer->material_uniforms, 
        material->shininess
    );

    upload_material_alpha(
        &renderer->material_uniforms, 
        material->alpha_mode, 
        material->alpha_cutoff
    );
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material->diffuse_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material->specular_texture);

    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
}

static void renderer_update_projection(
    struct RendererState *renderer,
    const Camera *camera,
    int width,
    int height
)
{
    glm_perspective(
        glm_rad(camera->cameraFOV),
        (float)width / (float)height,
        0.1f,
        100.0f,
        renderer->projection
    );
}

static bool renderer_resize_for_viewport(
    struct RendererState *renderer,
    const Camera *camera,
    RendererViewport viewport
)
{
    if (viewport.width <= 0 || viewport.height <= 0)
    {
        return false;
    }

    if (renderer->scene_target.width == viewport.width &&
        renderer->scene_target.height == viewport.height &&
        renderer->scene_msaa_target.width == viewport.width &&
        renderer->scene_msaa_target.height == viewport.height)
    {
        return true;
    }

    if (render_target_resize(
            &renderer->scene_target,
            viewport.width,
            viewport.height
        ) != 0)
    {
        return false;
    }

    if (msaa_render_target_resize(
            &renderer->scene_msaa_target,
            viewport.width,
            viewport.height
        ) != 0)
    {
        return false;
    }

    renderer_update_projection(
        renderer,
        camera,
        viewport.width,
        viewport.height
    );

    return true;
}

unsigned int renderer_get_resolved_scene_texture(const Renderer *renderer)
{
    if (renderer == NULL)
    {
        return 0;
    }

    return renderer->scene_target.color_texture;
}

void renderer_render_frame(Renderer *renderer, const RendererFrame *frame)
{
    if (renderer == NULL || frame == NULL || frame->camera == NULL)
    {
        return;
    }

    if (!renderer_resize_for_viewport(
            renderer,
            frame->camera,
            frame->viewport
        ))
    {
        return;
    }

    // Wipe drawing surface clear
    msaa_render_target_bind(&renderer->scene_msaa_target);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Put the shader program and VAO in focus in OpenGL's state machine
    glUseProgram(renderer->shader_program);

    upload_camera_ubo(renderer->camera_ubo, frame->camera, renderer->projection);

    upload_directional_light(frame->directional_light, &renderer->directional_light_uniforms);

    upload_point_light_collection(frame->point_lights, renderer->point_light_uniforms, renderer->point_light_count_location);

    upload_spot_light_collection(frame->spot_lights, renderer->spot_light_uniforms, renderer->spot_light_count_location);

    programmable_mesh_resources_begin_frame(
        &renderer->programmable_meshes
    );

    for (size_t i = 0; i < frame->renderable_count; i++)
    {
        const RenderableDrawData *renderable = &frame->renderables[i];

        mat4s model_matrix = renderable->model_matrix;

        switch (renderable->geometry_type)
        {
            case RENDERABLE_GEOMETRY_MODEL:
                if (renderer->loaded_model_path == NULL ||
                    renderable->model_path == NULL ||
                    strcmp(renderer->loaded_model_path, renderable->model_path) != 0)
                {
                    break;
                }

                draw_model(
                    &renderer->test_model,
                    renderer->model_location,
                    &renderer->material_uniforms,
                    model_matrix.raw,
                    frame->camera->cameraPos.raw
                );
                break;

            case RENDERABLE_GEOMETRY_PRIMITIVE:
            {
                const Mesh *mesh = primitive_mesh_resources_get(
                    &renderer->primitive_meshes, 
                    renderable->primitive_type
                    );

                draw_primitive_mesh(
                    renderer, 
                    mesh, 
                    renderable->model_matrix
                );
                break;
            }

            case RENDERABLE_GEOMETRY_PROGRAMMABLE:
            {
                if (!programmable_mesh_resources_sync(
                        &renderer->programmable_meshes, 
                        renderable->programmable_mesh_id, 
                        renderable->programmable_mesh
                    ))
                {
                    break;
                }

                const Mesh *mesh = programmable_mesh_resources_get(
                    &renderer->programmable_meshes, 
                    renderable->programmable_mesh_id
                );

                draw_primitive_mesh(
                    renderer, 
                    mesh, 
                    renderable->model_matrix
                );

                break;
            }
        }
    }

    programmable_mesh_resources_end_frame(
        &renderer->programmable_meshes
    );

    mat4s skybox_view = frame->camera->view;
    skybox_draw(&renderer->skybox, renderer->projection, skybox_view.raw);

    msaa_render_target_resolve_to(&renderer->scene_msaa_target, &renderer->scene_target);

    render_target_unbind();

    if (!frame->present_to_default_framebuffer)
    {
        return;
    }

    glViewport(0, 0, frame->viewport.width, frame->viewport.height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_screen_quad(renderer);
}

static int bind_camera_uniform_block(GLuint shader_program)
{
    GLuint camera_block_index = glGetUniformBlockIndex(shader_program, "CameraBlock");

    if (camera_block_index == GL_INVALID_INDEX)
    {
        fprintf(stderr, "Failed to find CameraBlock uniform block\n");
        return 1;
    }

    glUniformBlockBinding(shader_program, camera_block_index, CAMERA_UBO_BINDING);

    return 0;
}

static int init_shader_program(struct RendererState *renderer)
{
    GLuint vs, fs;

    if (load_shaders(&vs, &fs, "src/renderer/shaders/light.vert", "src/renderer/shaders/light.frag") != 0)
    {
        return 1;
    }

    if (create_shader_program(&vs, &fs, &renderer->shader_program) != 0)
    {
        return 1;
    }

    if (bind_camera_uniform_block(renderer->shader_program) != 0)
    {
        return 1;
    }

    glUseProgram(renderer->shader_program);

    return 0;
}

static void init_camera_projection(struct RendererState *renderer, const RendererConfig *config)
{
    renderer_update_projection(
        renderer,
        config->camera,
        config->viewport.width,
        config->viewport.height
    );

    camera_ubo_init(&renderer->camera_ubo);

    upload_camera_ubo(renderer->camera_ubo, config->camera, renderer->projection);
}

static void init_material(struct RendererState *renderer)
{
    glUseProgram(renderer->shader_program);
    material_uniforms_init(&renderer->material_uniforms, renderer->shader_program);
    upload_material_samplers(&renderer->material_uniforms);
}

static int init_primitive_material(struct RendererState *renderer)
{
    renderer->primitive_material.diffuse_texture = 0;
    renderer->primitive_material.specular_texture = 0;
    renderer->primitive_material.diffuse_color =
        (vec4s){{1.0f, 1.0f, 1.0f, 1.0f}};
    renderer->primitive_material.shininess = 96.0f;
    renderer->primitive_material.alpha_mode = ALPHA_MODE_OPAQUE;
    renderer->primitive_material.alpha_cutoff = 0.5f;
    renderer->primitive_material.double_sided = false;

    if (create_solid_color_texture(
                &renderer->primitive_material.diffuse_texture, 
                255, 
                255, 
                255, 
                255) != 0)
    {
        fprintf(stderr, "Failed to create primitive default texture\n");
        return 1;
    }

    return 0;
}

static int init_screen_quad(struct RendererState *renderer)
{
    float quad_vertices[] = {
        // positions | texcoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    GLuint vs, fs;

    if (load_shaders(&vs, &fs, "src/renderer/shaders/screen.vert", "src/renderer/shaders/screen.frag") != 0)
    {
        return 1;
    }

    if (create_shader_program(&vs, &fs, &renderer->screen_shader_program) != 0)
    {
        return 1;
    }

    glGenVertexArrays(1, &renderer->screen_quad_vao);
    glGenBuffers(1, &renderer->screen_quad_vbo);

    glBindVertexArray(renderer->screen_quad_vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->screen_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindVertexArray(0);

    glUseProgram(renderer->screen_shader_program);

    renderer->screen_texture_location = glGetUniformLocation(renderer->screen_shader_program, "screenTexture");

    glUniform1i(renderer->screen_texture_location, 0);

    return 0;
}

static void init_lights(struct RendererState *renderer, const RendererConfig *config)
{

    directional_light_uniforms_init(&renderer->directional_light_uniforms, renderer->shader_program);

    renderer->point_light_count_location = glGetUniformLocation(renderer->shader_program, "pointLightCount");

    for (size_t i = 0; i < MAX_SHADER_POINT_LIGHTS; i++)
    {
        point_light_uniforms_init(&renderer->point_light_uniforms[i], renderer->shader_program, i);
    }

    renderer->spot_light_count_location = glGetUniformLocation(renderer->shader_program, "spotLightCount");

    for (size_t j = 0; j < MAX_SHADER_SPOT_LIGHTS; j++)
    {
        spot_light_uniforms_init(&renderer->spot_light_uniforms[j], renderer->shader_program, j);
    }
}

static int renderer_state_init(struct RendererState *renderer, const RendererConfig *config)
{
    primitive_mesh_resources_init(&renderer->primitive_meshes);
    programmable_mesh_resources_init(&renderer->programmable_meshes);
    if (init_shader_program(renderer) != 0)
    {
        return 1;
    }

    init_lights(renderer, config);

    if (render_target_init(&renderer->scene_target, config->viewport.width, config->viewport.height) != 0)
    {
        return 1;
    }

    if (msaa_render_target_init(&renderer->scene_msaa_target, config->viewport.width, config->viewport.height, 4) != 0)
    {
        return 1;
    }

    if (init_screen_quad(renderer) != 0)
    {
        return 1;
    }

    renderer->loaded_model_path = NULL;

    if (config->model_path != NULL && config->model_path[0] != '\0')
    {
        if (model_load_gltf(&renderer->test_model, config->model_path))
        {
            return 1;
        }

        renderer->loaded_model_path = config->model_path;
    }

    init_camera_projection(renderer, config);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    renderer->model_location = glGetUniformLocation(renderer->shader_program, "model");

    if (renderer->model_location < 0)
    {
        fprintf(stderr, "Failed to get uniform location");
        return 1;
    }

    init_material(renderer);

    if (init_primitive_material(renderer) != 0)
    {
        return 1;
    }

    if (skybox_init(&renderer->skybox, config->skybox_faces) != 0)
    {
        fprintf(stderr, "Failed to initialize skybox\n");
        return 1;
    }

    if (!primitive_mesh_resources_upload(&renderer->primitive_meshes))
    {
        fprintf(stderr, "Failed to upload primitive_meshes\n");
        return 1;
    }

    return 0;
}

static void renderer_state_shutdown(struct RendererState *renderer)
{
    programmable_mesh_resources_free(&renderer->programmable_meshes);
    primitive_mesh_resources_free(&renderer->primitive_meshes);

    if (renderer->primitive_material.diffuse_texture != 0)
    {
        glDeleteTextures(1, &renderer->primitive_material.diffuse_texture);
        renderer->primitive_material.diffuse_texture = 0;
    }

    model_free(&renderer->test_model);
    // instanced_model_free(&renderer->instance_instances);
    skybox_free(&renderer->skybox);

    msaa_render_target_free(&renderer->scene_msaa_target);
    render_target_free(&renderer->scene_target);

    if (renderer->screen_quad_vbo)
    {
        glDeleteBuffers(1, &renderer->screen_quad_vbo);
    }

    if (renderer->screen_quad_vao)
    {
        glDeleteVertexArrays(1, &renderer->screen_quad_vao);
    }

    if (renderer->screen_shader_program)
    {
        glDeleteProgram(renderer->screen_shader_program);
    }

    if (renderer->camera_ubo)
    {
        glDeleteBuffers(1, &renderer->camera_ubo);
    }
    glDeleteProgram(renderer->shader_program);
}

int renderer_init(Renderer *renderer, const RendererConfig *config)
{
    if (renderer_state_init(renderer, config) != 0)
    {
        renderer_state_shutdown(renderer);
        fprintf(stderr, "Failed to initialize renderer state\n");
        return 1;
    }

    return 0;
}

void renderer_shutdown(Renderer *renderer)
{
    renderer_state_shutdown(renderer);
}

Renderer *renderer_create(void)
{
    return calloc(1, sizeof(Renderer));
}

void renderer_destroy(Renderer *renderer)
{
    free(renderer);
}
