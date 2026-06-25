#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/struct.h>

#include "../controller/input.h"
#include "data_types/mesh.h"

#include "data_types/texture.h"
#include "data_types/renderObject.h"
#include "data_types/lightObject.h"
#include "data_types/material.h"

#include "shaders.h"
#include "camera.h"

#define ACTIVE_SPOT_LIGHTS 2
#define ACTIVE_POINT_LIGHTS 4
struct RendererState
{
    RenderObjectArray render_objects;
    mat4 projection;
    Camera camera;
    DirectionalLight directional_light;
    PointLightCollection point_lights;
    SpotLightCollection spot_lights;

    DirectionalLightUniforms directional_light_uniforms;
    PointLightUniforms point_light_uniforms[MAX_POINT_LIGHTS];
    SpotLightUniforms spot_light_uniforms[MAX_SPOT_LIGHTS];
    MaterialUniforms material_uniforms;

    GLuint shader_program;
    GLint point_light_count_location;
    GLint spot_light_count_location;
    GLint model_location;
    GLint projection_location;
    GLint view_location;
    GLint view_pos_location;
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

LightColor sunlight = {
    .ambient = {{0.05f, 0.05f, 0.05f}},
    .diffuse = {{0.4f, 0.4f, 0.4f}},
    .specular = {{0.5f, 0.5f, 0.5f}}};

LightColor point_light_colors[MAX_POINT_LIGHTS] = {
    {.ambient = {{0.02f, 0.002f, 0.002f}},
     .diffuse = {{1.0f, 0.1f, 0.1f}},
     .specular = {{1.0f, 0.3f, 0.3f}}},
    {.ambient = {{0.002f, 0.02f, 0.002f}},
     .diffuse = {{0.1f, 1.0f, 0.1f}},
     .specular = {{0.3f, 1.0f, 0.3f}}},
    {.ambient = {{0.002f, 0.002f, 0.02f}},
     .diffuse = {{0.1f, 0.1f, 1.0f}},
     .specular = {{0.3f, 0.3f, 1.0f}}},
    {.ambient = {{0.02f, 0.0154f, 0.0112f}},
     .diffuse = {{1.0f, 0.77f, 0.56f}},
     .specular = {{1.0f, 0.77f, 0.56f}}}};

vec3s point_light_positions[] = {
    {{0.7f, 0.2f, 2.0f}},
    {{2.3f, -3.3f, -4.0f}},
    {{-4.0f, 2.0f, -12.0f}},
    {{0.0f, 0.0f, -3.0f}}};

LightColor spot_light_colors[MAX_SPOT_LIGHTS] = {
    {// Hot magenta/pink
     .ambient = {{0.0f, 0.0f, 0.0f}},
     .diffuse = {{4.0f, 0.0f, 2.5f}},
     .specular = {{4.0f, 0.0f, 2.5f}}},
    {// Electric cyan/blue
     .ambient = {{0.0f, 0.0f, 0.0f}},
     .diffuse = {{0.0f, 3.0f, 4.0f}},
     .specular = {{0.0f, 3.0f, 4.0f}}}};

vec3s spot_light_positions[] = {
    {{0.0f, 15.0f, -7.5f}},
    {{4.0f, 12.0f, -3.0f}}};

vec3s spot_light_directions[] = {
    {{0.0f, -1.0f, 0.0f}},
    {{-0.3f, -1.0f, -0.2f}}};

static void fps_counter(double *delta_time, double *title_countdown_time, GLFWwindow *window)
{
    *title_countdown_time -= *delta_time;
    if (*title_countdown_time <= 0.0 && *delta_time > 0.0)
    {
        double fps = 1.0 / *delta_time;

        // Create a string and put the FPS as the window title.
        char title[256];
        sprintf(title, "FPS = %.2lf", fps);
        glfwSetWindowTitle(window, title);
        *title_countdown_time = 0.1;
    }
}

static void update_frame_time(double current_time, double *previous_time, double *delta_time)
{
    *delta_time = current_time - *previous_time;
    *previous_time = current_time;
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    vec2s offsets = input_get_mouse_offsets(xpos, ypos);
    handle_mouse(&renderer.camera, offsets, true);
}

static void run_render_loop(GLFWwindow *window, bool fps_enabled, struct RendererState *renderer_state)
{
    double previous_time = glfwGetTime();
    double title_countdown_time = 0.1;
    double delta_time = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        double current_time = glfwGetTime();
        // Update delta time and previous time for next frame
        update_frame_time(current_time, &previous_time, &delta_time);

        if (fps_enabled)
        {
            fps_counter(&delta_time, &title_countdown_time, window);
        }

        // Update window events
        glfwPollEvents();

        // Wipe drawing surface clear
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: Camera movement
        vec2s movement_axis = input_get_movement_axis();
        camera_movement(&renderer_state->camera, movement_axis, delta_time);
        camera_update(&renderer_state->camera);

        // Put the shader program and VAO in focus in OpenGL's state machine
        glUseProgram(renderer_state->shader_program);

        upload_material_shininess(&renderer_state->material_uniforms, 32.0f);

        upload_camera(renderer_state->view_location, renderer_state->view_pos_location, &renderer_state->camera);

        upload_directional_light(&renderer_state->directional_light, &renderer_state->directional_light_uniforms);

        upload_point_light_collection(&renderer_state->point_lights, renderer_state->point_light_uniforms, renderer_state->point_light_count_location);

        upload_spot_light_collection(&renderer_state->spot_lights, renderer_state->spot_light_uniforms, renderer_state->spot_light_count_location);

        vec3s rotation_axis = {1.0f, 0.3f, 0.5f};

        for (int i = 0; i < renderer_state->render_objects.count; i++)
        {
            identity_model(&renderer_state->render_objects.items[i]);
            translate_model_matrix(&renderer_state->render_objects.items[i], renderer_state->render_objects.items[i].position);
            rotate_model(&renderer_state->render_objects.items[i], glm_rad(20.0f * (i + 1) * current_time), rotation_axis);

            draw_render_object(&renderer_state->render_objects.items[i], renderer_state->model_location);
        }

        // Put the drawing into the visible area
        glfwSwapBuffers(window);
    }
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
        if (create_texture(&new_render_object.mesh, "assets/diffuse maps/container2.png") != 0)
        {
            return 1;
        }
        if (create_texture(&new_render_object.mesh, "assets/specular maps/container2_specular.png") != 0)
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

    glUseProgram(renderer->shader_program);

    return 0;
}

static void init_lighting(struct RendererState *renderer)
{
    directional_light_init(&renderer->directional_light, (vec3s){{-0.2f, -1.0f, -0.3f}}, sunlight);

    directional_light_uniforms_init(&renderer->directional_light_uniforms, renderer->shader_program);

    point_light_collection_init(&renderer->point_lights);

    for (size_t i = 0; i < ACTIVE_POINT_LIGHTS; i++)
    {
        PointLight light = {0};

        point_light_init(&light, point_light_positions[i], point_light_colors[i], 1.0f, 0.09f, 0.032f);

        point_light_collection_add(&renderer->point_lights, light);
    }

    renderer->point_light_count_location = glGetUniformLocation(renderer->shader_program, "pointLightCount");

    for (size_t i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        point_light_uniforms_init(&renderer->point_light_uniforms[i], renderer->shader_program, i);
    }

    spot_light_collection_init(&renderer->spot_lights);

    for (size_t i = 0; i < ACTIVE_SPOT_LIGHTS; i++)
    {
        SpotLight light = {0};

        spot_light_init(&light, spot_light_positions[i], spot_light_directions[i], spot_light_colors[i], 1.0f, 0.045f, 0.0075f, 30.0f, 40.0f);

        spot_light_collection_add(&renderer->spot_lights, light);
    }

    renderer->spot_light_count_location =
        glGetUniformLocation(renderer->shader_program, "spotLightCount");

    for (size_t i = 0; i < MAX_SPOT_LIGHTS; i++)
    {
        spot_light_uniforms_init(&renderer->spot_light_uniforms[i], renderer->shader_program, i);
    }
}

static void init_camera_projection(struct RendererState *renderer)
{
    camera_init(&renderer->camera);

    renderer->projection_location = glGetUniformLocation(renderer->shader_program, "projection");
    glm_perspective(glm_rad(renderer->camera.cameraFOV), 800.0f / 600.0f, 0.1f, 100.0f, renderer->projection);
    glUniformMatrix4fv(renderer->projection_location, 1, GL_FALSE, (float *)renderer->projection);
    renderer->view_location = glGetUniformLocation(renderer->shader_program, "view");

    renderer->view_pos_location = glGetUniformLocation(renderer->shader_program, "viewPos");
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

static int renderer_init(struct RendererState *renderer)
{
    if (init_render_objects(renderer) != 0)
    {
        return 1;
    }

    if (init_shader_program(renderer) != 0)
    {
        return 1;
    }

    init_lighting(renderer);

    init_camera_projection(renderer);

    glEnable(GL_DEPTH_TEST);

    renderer->model_location = glGetUniformLocation(renderer->shader_program, "model");

    if (renderer->model_location < 0)
    {
        fprintf(stderr, "Failed to get uniform location");
        return 1;
    }

    init_material(renderer);

    init_scene_positions(renderer);

    return 0;
}

static void renderer_shutdown(struct RendererState *renderer)
{
    for (int i = 0; i < renderer->render_objects.count; i++)
    {
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.position_vbo);
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.color_vbo);
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.uv_vbo);
        glDeleteTextures(1, &renderer->render_objects.items[i].mesh.texture);
        glDeleteTextures(1, &renderer->render_objects.items[i].mesh.texture2);
        glDeleteVertexArrays(1, &renderer->render_objects.items[i].mesh.vao);
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.ebo);
    }
    free_renderobject_array(&renderer->render_objects);
    glDeleteProgram(renderer->shader_program);
}

int renderer_run(GLFWwindow *window, bool fps_enabled)
{

    if (renderer_init(&renderer) != 0)
    {
        renderer_shutdown(&renderer);
        fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }
    run_render_loop(window, fps_enabled, &renderer);
    renderer_shutdown(&renderer);
    return 0;
}
