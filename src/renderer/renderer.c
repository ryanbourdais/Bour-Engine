#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/struct.h>

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

#define ACTIVE_SPOT_LIGHTS 2
#define ACTIVE_POINT_LIGHTS 4
struct RendererState
{
    RenderObjectArray render_objects;
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
    // GLint projection_location;
    // GLint view_location;
    // GLint view_pos_location;

    GLint use_instancing_location;
};

struct RendererState renderer = {0};

Vertex cube[] = {
    {.position = {-0.5f, -0.5f, 0.5f}, .color = {1.0f, 0.0f, 0.0f}, .uv = {0.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f}},
    {.position = {0.5f, -0.5f, 0.5f}, .color = {0.0f, 1.0f, 0.0f}, .uv = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f}},
    {.position = {0.5f, 0.5f, 0.5f}, .color = {0.0f, 0.0f, 1.0f}, .uv = {1.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f}},
    {.position = {-0.5f, 0.5f, 0.5f}, .color = {1.0f, 1.0f, 0.0f}, .uv = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f}},

    {.position = {0.5f, -0.5f, 0.5f}, .color = {0.0f, 1.0f, 1.0f}, .uv = {0.0f, 0.0f}, .normal = {1.0f, 0.0f, 0.0f}},
    {.position = {0.5f, -0.5f, -0.5f}, .color = {1.0f, 0.0f, 1.0f}, .uv = {1.0f, 0.0f}, .normal = {1.0f, 0.0f, 0.0f}},
    {.position = {0.5f, 0.5f, -0.5f}, .color = {1.0f, 1.0f, 1.0f}, .uv = {1.0f, 1.0f}, .normal = {1.0f, 0.0f, 0.0f}},
    {.position = {0.5f, 0.5f, 0.5f}, .color = {0.2f, 0.2f, 0.2f}, .uv = {0.0f, 1.0f}, .normal = {1.0f, 0.0f, 0.0f}},

    {.position = {0.5f, -0.5f, -0.5f}, .color = {1.0f, 0.5f, 0.0f}, .uv = {0.0f, 0.0f}, .normal = {0.0f, 0.0f, -1.0f}},
    {.position = {-0.5f, -0.5f, -0.5f}, .color = {0.5f, 1.0f, 0.0f}, .uv = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, -1.0f}},
    {.position = {-0.5f, 0.5f, -0.5f}, .color = {0.0f, 0.5f, 1.0f}, .uv = {1.0f, 1.0f}, .normal = {0.0f, 0.0f, -1.0f}},
    {.position = {0.5f, 0.5f, -0.5f}, .color = {1.0f, 0.0f, 0.5f}, .uv = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, -1.0f}},

    {.position = {-0.5f, -0.5f, -0.5f}, .color = {0.7f, 0.2f, 0.2f}, .uv = {0.0f, 0.0f}, .normal = {-1.0f, 0.0f, 0.0f}},
    {.position = {-0.5f, -0.5f, 0.5f}, .color = {0.2f, 0.7f, 0.2f}, .uv = {1.0f, 0.0f}, .normal = {-1.0f, 0.0f, 0.0f}},
    {.position = {-0.5f, 0.5f, 0.5f}, .color = {0.2f, 0.2f, 0.7f}, .uv = {1.0f, 1.0f}, .normal = {-1.0f, 0.0f, 0.0f}},
    {.position = {-0.5f, 0.5f, -0.5f}, .color = {0.7f, 0.7f, 0.2f}, .uv = {0.0f, 1.0f}, .normal = {-1.0f, 0.0f, 0.0f}},

    {.position = {-0.5f, 0.5f, 0.5f}, .color = {0.7f, 0.2f, 0.7f}, .uv = {0.0f, 0.0f}, .normal = {0.0f, 1.0f, 0.0f}},
    {.position = {0.5f, 0.5f, 0.5f}, .color = {0.2f, 0.7f, 0.7f}, .uv = {1.0f, 0.0f}, .normal = {0.0f, 1.0f, 0.0f}},
    {.position = {0.5f, 0.5f, -0.5f}, .color = {0.8f, 0.8f, 0.8f}, .uv = {1.0f, 1.0f}, .normal = {0.0f, 1.0f, 0.0f}},
    {.position = {-0.5f, 0.5f, -0.5f}, .color = {0.3f, 0.3f, 0.3f}, .uv = {0.0f, 1.0f}, .normal = {0.0f, 1.0f, 0.0f}},

    {.position = {-0.5f, -0.5f, -0.5f}, .color = {0.9f, 0.4f, 0.4f}, .uv = {0.0f, 0.0f}, .normal = {0.0f, -1.0f, 0.0f}},
    {.position = {0.5f, -0.5f, -0.5f}, .color = {0.4f, 0.9f, 0.4f}, .uv = {1.0f, 0.0f}, .normal = {0.0f, -1.0f, 0.0f}},
    {.position = {0.5f, -0.5f, 0.5f}, .color = {0.4f, 0.4f, 0.9f}, .uv = {1.0f, 1.0f}, .normal = {0.0f, -1.0f, 0.0f}},
    {.position = {-0.5f, -0.5f, 0.5f}, .color = {0.9f, 0.9f, 0.4f}, .uv = {0.0f, 1.0f}, .normal = {0.0f, -1.0f, 0.0f}}};

unsigned int cube_indices[] = {
    0, 1, 2, 0, 2, 3,
    4, 5, 6, 4, 6, 7,
    8, 9, 10, 8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23};

size_t cube_vertex_count = sizeof(cube) / sizeof(cube[0]);
GLsizei cube_index_count = sizeof(cube_indices) / sizeof(cube_indices[0]);

LightColor debug_sunlight = {
    .ambient  = {{0.02f, 0.02f, 0.02f}},
    .diffuse  = {{0.05f, 0.05f, 0.05f}},
    .specular = {{0.02f, 0.02f, 0.02f}}
};
    
LightColor sunlight = {
    .ambient = {{0.18f, 0.18f, 0.18f}},
    .diffuse = {{0.65f, 0.62f, 0.56f}},
    .specular = {{0.25f, 0.25f, 0.25f}}
};

LightColor debug_point_light_colors[MAX_SHADER_POINT_LIGHTS] = {
    {
        .ambient  = {{0.00f, 0.00f, 0.00f}},
        .diffuse  = {{3.00f, 0.20f, 0.20f}},
        .specular = {{3.00f, 0.20f, 0.20f}}
    },
    {
        .ambient  = {{0.00f, 0.00f, 0.00f}},
        .diffuse  = {{0.20f, 3.00f, 0.20f}},
        .specular = {{0.20f, 3.00f, 0.20f}}
    },
    {
        .ambient  = {{0.00f, 0.00f, 0.00f}},
        .diffuse  = {{0.20f, 0.20f, 3.00f}},
        .specular = {{0.20f, 0.20f, 3.00f}}
    },
    {
        .ambient  = {{0.00f, 0.00f, 0.00f}},
        .diffuse  = {{3.00f, 1.80f, 0.40f}},
        .specular = {{3.00f, 1.80f, 0.40f}}
    }
};

LightColor point_light_colors[MAX_SHADER_POINT_LIGHTS] = {
    {
        .ambient = {{0.01f, 0.01f, 0.01f}},
        .diffuse = {{0.75f, 0.70f, 0.62f}},
        .specular = {{0.25f, 0.24f, 0.22f}}
    },
    {
        .ambient = {{0.005f, 0.005f, 0.006f}},
        .diffuse = {{0.35f, 0.38f, 0.45f}},
        .specular = {{0.12f, 0.13f, 0.16f}}
    },
    {
        .ambient = {{0.005f, 0.005f, 0.005f}},
        .diffuse = {{0.28f, 0.30f, 0.32f}},
        .specular = {{0.08f, 0.08f, 0.08f}}
    },
    {
        .ambient = {{0.005f, 0.004f, 0.003f}},
        .diffuse = {{0.45f, 0.36f, 0.28f}},
        .specular = {{0.12f, 0.10f, 0.08f}}
    }
};


vec3s point_light_positions[] = {
    {{ 0.0f,  2.5f,  0.0f}},
    {{ 3.0f,  2.0f,  0.0f}},
    {{-3.0f,  2.0f,  0.0f}},
    {{ 0.0f,  2.0f, -3.0f}}
};

LightColor debug_spot_light_colors[MAX_SHADER_SPOT_LIGHTS] = {
    {// Hot magenta/pink
     .ambient = {{0.0f, 0.0f, 0.0f}},
     .diffuse = {{4.0f, 0.0f, 2.5f}},
     .specular = {{4.0f, 0.0f, 2.5f}}},
    {// Electric cyan/blue
     .ambient = {{0.0f, 0.0f, 0.0f}},
     .diffuse = {{0.0f, 3.0f, 4.0f}},
     .specular = {{0.0f, 3.0f, 4.0f}}}};

LightColor spot_light_colors[MAX_SHADER_SPOT_LIGHTS] = {
    {
        .ambient = {{0.0f, 0.0f, 0.0f}},
        .diffuse = {{0.65f, 0.60f, 0.52f}},
        .specular = {{0.22f, 0.20f, 0.18f}}
    },
    {
        .ambient = {{0.0f, 0.0f, 0.0f}},
        .diffuse = {{0.25f, 0.28f, 0.34f}},
        .specular = {{0.08f, 0.09f, 0.11f}}
    }
};

vec3s spot_light_positions[] = {
    {{ 0.0f, 3.0f,  2.0f}},
    {{ 0.0f, 3.0f, -2.0f}}
};

vec3s spot_light_directions[] = {
    {{ 0.0f, -1.0f, -0.3f}},
    {{ 0.0f, -1.0f,  0.3f}}
};


const char *skybox_faces[6] = {
    "assets/cubemaps/skybox/right.jpg",
    "assets/cubemaps/skybox/left.jpg",
    "assets/cubemaps/skybox/top.jpg",
    "assets/cubemaps/skybox/bottom.jpg",
    "assets/cubemaps/skybox/front.jpg",
    "assets/cubemaps/skybox/back.jpg"
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

void renderer_render_frame(GLFWwindow *window, const Camera *camera)
{
    struct RendererState *renderer_state = &renderer;
    
    // Wipe drawing surface clear
    msaa_render_target_bind(&renderer_state->scene_msaa_target);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Put the shader program and VAO in focus in OpenGL's state machine
    glUseProgram(renderer_state->shader_program);

    upload_camera_ubo(renderer_state->camera_ubo, camera, renderer_state->projection);

    upload_directional_light(&renderer_state->directional_light, &renderer_state->directional_light_uniforms);

    upload_point_light_collection(&renderer_state->point_lights, renderer_state->point_light_uniforms, renderer_state->point_light_count_location);

    upload_spot_light_collection(&renderer_state->spot_lights, renderer_state->spot_light_uniforms, renderer_state->spot_light_count_location);

    mat4 model_matrix;
    glm_mat4_identity(model_matrix);

    glUniform1i(renderer_state->use_instancing_location, 0);
    draw_model(&renderer_state->test_model, renderer_state->model_location, &renderer_state->material_uniforms, model_matrix , camera->cameraPos.raw);

    msaa_render_target_resolve_to(&renderer_state->scene_msaa_target, &renderer_state->scene_target);

    render_target_unbind();

    int framebuffer_width = 0;
    int framebuffer_height = 0;

    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

    glViewport(0, 0, framebuffer_width, framebuffer_height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_screen_quad(renderer_state);
}

static int init_render_objects(struct RendererState *renderer)
{
    renderobject_array_initialize(&renderer->render_objects);
    for (int i = 0; i < 10; i++)
    {
        RenderObject new_render_object = {0};

        int mesh_status = create_mesh_from_vertices(&new_render_object.mesh, cube, cube_vertex_count, cube_indices, cube_index_count);
        if (mesh_status != 0)
        {
            fprintf(stderr, "Mesh failed to be created, exiting!");
            return 1;
        }
        if (create_texture_ex(&new_render_object.mesh, "assets/diffuse maps/container2.png", true) != 0)
        {
            return 1;
        }
        if (create_texture_ex(&new_render_object.mesh, "assets/specular maps/container2_specular.png", false) != 0)
        {
            return 1;
        }
        renderobject_array_append(&renderer->render_objects, new_render_object);
    }
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

static void init_lighting(struct RendererState *renderer)
{
    directional_light_init(&renderer->directional_light, (vec3s){{-0.2f, -1.0f, -0.3f}}, debug_sunlight);

    directional_light_uniforms_init(&renderer->directional_light_uniforms, renderer->shader_program);

    point_light_collection_init(&renderer->point_lights);

    for (size_t i = 0; i < ACTIVE_POINT_LIGHTS; i++)
    {
        PointLight light = {0};

        point_light_init(&light, point_light_positions[i], debug_point_light_colors[i], 1.0f, 0.09f, 0.032f);

        point_light_collection_add(&renderer->point_lights, light);
    }

    renderer->point_light_count_location = glGetUniformLocation(renderer->shader_program, "pointLightCount");

    for (size_t i = 0; i < MAX_SHADER_POINT_LIGHTS; i++)
    {
        point_light_uniforms_init(&renderer->point_light_uniforms[i], renderer->shader_program, i);
    }

    spot_light_collection_init(&renderer->spot_lights);

    for (size_t i = 0; i < ACTIVE_SPOT_LIGHTS; i++)
    {
        SpotLight light = {0};

        spot_light_init(&light, spot_light_positions[i], spot_light_directions[i], debug_spot_light_colors[i], 1.0f, 0.09f, 0.032f, 25.0f, 45.0f);

        spot_light_collection_add(&renderer->spot_lights, light);
    }

    renderer->spot_light_count_location =
        glGetUniformLocation(renderer->shader_program, "spotLightCount");

    for (size_t i = 0; i < MAX_SHADER_SPOT_LIGHTS; i++)
    {
        spot_light_uniforms_init(&renderer->spot_light_uniforms[i], renderer->shader_program, i);
    }
}

static void init_camera_projection(struct RendererState *renderer, GLFWwindow *window, const Camera *camera)
{

    int framebuffer_width = 0;
    int framebuffer_height = 0;

    glfwGetFramebufferSize(
        window,
        &framebuffer_width,
        &framebuffer_height
    );

    glm_perspective(glm_rad(camera->cameraFOV), (float)framebuffer_width / (float)framebuffer_height, 0.1f, 100.0f, renderer->projection);

    camera_ubo_init(&renderer->camera_ubo);

    upload_camera_ubo(renderer->camera_ubo, camera, renderer->projection);
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

static void init_material(struct RendererState *renderer)
{
    material_uniforms_init(&renderer->material_uniforms, renderer->shader_program);
    upload_material_samplers(&renderer->material_uniforms);
}

static void init_scene_positions(struct RendererState *renderer)
{
    vec3s cubePositions[] = {
        (vec3s){{0.0f, 0.0f, 0.0f}},
        (vec3s){{2.0f, 5.0f, -15.0f}},
        (vec3s){{-1.5f, -2.2f, -2.5f}},
        (vec3s){{-3.8f, -2.0f, -12.3f}},
        (vec3s){{2.4f, -0.4f, -3.5f}},
        (vec3s){{-1.7f, 3.0f, -7.5f}},
        (vec3s){{1.3f, -2.0f, -2.5f}},
        (vec3s){{1.5f, 2.0f, -2.5f}},
        (vec3s){{1.5f, 0.2f, -1.5f}},
        (vec3s){{-1.3f, 1.0f, -1.5f}}};

    for (int i = 0; i < renderer->render_objects.count; i++)
    {
        renderer->render_objects.items[i].position = cubePositions[i];
        renderer->render_objects.items[i].rotation_angle = 0.0f;
        renderer->render_objects.items[i].scale = (vec3s){{1.0f, 1.0f, 1.0f}};
    }
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

static int renderer_state_init(struct RendererState *renderer, GLFWwindow *window, const Camera *camera)
{
    if (init_shader_program(renderer) != 0)
    {
        return 1;
    }

    int framebuffer_width = 0;
    int framebuffer_height = 0;

    glfwGetFramebufferSize(
        window,
        &framebuffer_width,
        &framebuffer_height
    );


    if (render_target_init(&renderer->scene_target, framebuffer_width, framebuffer_height) != 0)
    {
        return 1;
    }

    if (msaa_render_target_init(&renderer->scene_msaa_target, framebuffer_width, framebuffer_height, 4) != 0)
    {
        return 1;
    }

    if (init_screen_quad(renderer) != 0)
    {
        return 1;
    }

    if (model_load_gltf(&renderer->test_model, "assets/models/loft_japanese_11_free_interior/scene.gltf") != 0)
    {
        fprintf(stderr, "Failed to load loft interior model\n");
        return 1;
    }


    init_lighting(renderer);

    init_camera_projection(renderer, window, camera);

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

    if (skybox_init(&renderer->skybox, skybox_faces) != 0)
    {
        fprintf(stderr, "Failed to initialize skybox\n");
        return 1;
    }
    init_scene_positions(renderer);

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

int renderer_init(GLFWwindow *window, const Camera *camera)
{
    if (renderer_state_init(&renderer, window, camera) != 0)
    {
        renderer_state_shutdown(&renderer);
        fprintf(stderr, "Failed to initialize renderer state\n");
        return 1;
    }

    return 0;
}

void renderer_shutdown(void)
{
    renderer_state_shutdown(&renderer);
}
