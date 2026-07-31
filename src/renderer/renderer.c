#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/struct.h>
#include <cglm/mat4.h>

#include "data_types/mesh.h"
#include "data_types/texture.h"
#include "data_types/renderObject.h"
#include "data_types/lightObject.h"
#include "data_types/material.h"
#include "data_types/model.h"
#include "data_types/skybox.h"
#include "data_types/instancedModel.h"
#include "data_types/renderTarget.h"

#include "shaders.h"

struct RendererState
{
    mat4s model_matrix;

    mat4 projection;
    Model test_model;
    Skybox skybox;
    DirectionalLight directional_light;
    PointLightCollection point_lights;
    SpotLightCollection spot_lights;
    Model instance_model;
    InstancedModel instance_instances;
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

    GLint use_instancing_location;
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

void renderer_render_frame(Renderer *renderer, const RendererFrame *frame)
{
    // Wipe drawing surface clear
    msaa_render_target_bind(&renderer->scene_msaa_target);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Put the shader program and VAO in focus in OpenGL's state machine
    glUseProgram(renderer->shader_program);

    upload_camera_ubo(renderer->camera_ubo, frame->camera, renderer->projection);

    upload_directional_light(&renderer->directional_light, &renderer->directional_light_uniforms);

    upload_point_light_collection(&renderer->point_lights, renderer->point_light_uniforms, renderer->point_light_count_location);

    upload_spot_light_collection(&renderer->spot_lights, renderer->spot_light_uniforms, renderer->spot_light_count_location);


    glUniform1i(renderer->use_instancing_location, 0);
    draw_model(&renderer->test_model, renderer->model_location, &renderer->material_uniforms, renderer->model_matrix.raw , frame->camera->cameraPos.raw);

    msaa_render_target_resolve_to(&renderer->scene_msaa_target, &renderer->scene_target);

    render_target_unbind();

    glViewport(0, 0, frame->viewport.width, frame->viewport.height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_screen_quad(renderer);
}

static int bind_camera_uniform_block(GLuint shader_program)
{
    GLuint camera_block_index = glGetUniformBlockIndex(shader_program, "CameraBlock");

    if(camera_block_index == GL_INVALID_INDEX)
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

    glm_perspective(glm_rad(config->camera->cameraFOV), (float)config->viewport.width / (float)config->viewport.height, 0.1f, 100.0f, renderer->projection);

    camera_ubo_init(&renderer->camera_ubo);

    upload_camera_ubo(renderer->camera_ubo, config->camera, renderer->projection);
}

static void init_material(struct RendererState *renderer)
{
    glUseProgram(renderer->shader_program);
    material_uniforms_init(&renderer->material_uniforms, renderer->shader_program);
    upload_material_samplers(&renderer->material_uniforms);
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
    renderer->directional_light = *config->directional_light;
    renderer->point_lights = *config->point_lights;
    renderer->spot_lights = *config->spot_lights;

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

    const RenderableDrawData *renderable = NULL;

    if (config->renderable_count > 0)
    {
        renderable = &config->renderables[0];
    }

    const char *model_path = renderable != NULL ? renderable->model_path : config->model_path;

    if (model_load_gltf(&renderer->test_model, model_path) != 0)
    {
        return 1;
    }

    if (renderable != NULL)
    {
        renderer->model_matrix = renderable->model_matrix;
    }
    else {
        glm_mat4_identity(renderer->model_matrix.raw);
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

    renderer->use_instancing_location = glGetUniformLocation(renderer->shader_program, "useInstancing");

    if (renderer->model_location < 0)
    {
        fprintf(stderr, "Failed to get uniform location");
        return 1;
    }

    init_material(renderer);

    if (skybox_init(&renderer->skybox, config->skybox_faces) != 0)
    {
        fprintf(stderr, "Failed to initialize skybox\n");
        return 1;
    }

    return 0;
}

static void renderer_state_shutdown(struct RendererState *renderer)
{
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