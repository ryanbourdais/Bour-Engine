#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/struct.h>

#include <math.h>

#include "../controller/input.h"
#include "data_types/mesh.h"

#include "data_types/texture.h"
#include "data_types/renderObject.h"
#include "data_types/lightObject.h"

#include "shaders.h"
#include "camera.h"

struct RendererState {
    RenderObjectArray render_objects;
    mat4 projection;
    mat4 view;
    Camera camera;
    DirectionalLight directional_light;
    PointLightCollection point_lights;
    SpotLight spot_light;
    
    DirectionalLightUniforms directional_light_uniforms;
    PointLightUniforms point_light_uniforms[MAX_POINT_LIGHTS];
    SpotLightUniforms spot_light_uniforms;

    GLuint shader_program;
    GLuint lamp_shader_program;
    GLint point_light_count_location;
    GLint model_location;
    GLint projection_location;
    GLint view_location;
    GLint light_pos_location;
    GLint light_color_location;
    GLint object_color_location;
    GLint view_pos_location;
    GLint lamp_model_location;
    GLint lamp_view_location;
    GLint lamp_projection_location;
    GLint lamp_light_color_location;
    GLint material_ambient_location;
    GLint material_diffuse_location;
    GLint material_specular_location;
    GLint material_shininess_location;
    GLint light_ambient_location;
    GLint light_diffuse_location;
    GLint light_specular_location;
};

struct RendererState renderer = {0};

// TODO: Move to RenderObject objects in a separate file.
Vertex square[] = {
    {
        .position = { 0.5f,  0.5f, 0.0f },
        .color    = { 1.0f,  0.0f, 0.0f },
        .uv       = { 1.0f,  1.0f}
    },
    {
        .position = { 0.5f, -0.5f, 0.0f },
        .color    = { 0.0f,  1.0f, 0.0f },
        .uv       = { 1.0f,  0.0f}
    },
    {
        .position = {-0.5f, -0.5f, 0.0f },
        .color    = { 0.0f,  0.0f, 1.0f },
        .uv       = { 0.0f,  0.0f}
    },
        {
        .position = {-0.5f, 0.5f, 0.0f },
        .color    = { 0.0f,  0.0f, 0.0f },
        .uv       = { 0.0f,  1.0f}
    }
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

size_t vertex_count = sizeof(square) / sizeof(square[0]);
GLsizei index_count = sizeof(indices) / sizeof(indices[0]);

Vertex cube[] = {
    { .position = { -0.5f, -0.5f,  0.5f }, .color = { 1.0f, 0.0f, 0.0f }, .uv = { 0.0f, 0.0f }, .normal = {  0.0f,  0.0f,  1.0f } },
    { .position = {  0.5f, -0.5f,  0.5f }, .color = { 0.0f, 1.0f, 0.0f }, .uv = { 1.0f, 0.0f }, .normal = {  0.0f,  0.0f,  1.0f } },
    { .position = {  0.5f,  0.5f,  0.5f }, .color = { 0.0f, 0.0f, 1.0f }, .uv = { 1.0f, 1.0f }, .normal = {  0.0f,  0.0f,  1.0f } },
    { .position = { -0.5f,  0.5f,  0.5f }, .color = { 1.0f, 1.0f, 0.0f }, .uv = { 0.0f, 1.0f }, .normal = {  0.0f,  0.0f,  1.0f } },

    { .position = {  0.5f, -0.5f,  0.5f }, .color = { 0.0f, 1.0f, 1.0f }, .uv = { 0.0f, 0.0f }, .normal = {  1.0f,  0.0f,  0.0f } },
    { .position = {  0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 1.0f }, .uv = { 1.0f, 0.0f }, .normal = {  1.0f,  0.0f,  0.0f } },
    { .position = {  0.5f,  0.5f, -0.5f }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 1.0f, 1.0f }, .normal = {  1.0f,  0.0f,  0.0f } },
    { .position = {  0.5f,  0.5f,  0.5f }, .color = { 0.2f, 0.2f, 0.2f }, .uv = { 0.0f, 1.0f }, .normal = {  1.0f,  0.0f,  0.0f } },

    { .position = {  0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.5f, 0.0f }, .uv = { 0.0f, 0.0f }, .normal = {  0.0f,  0.0f, -1.0f } },
    { .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.5f, 1.0f, 0.0f }, .uv = { 1.0f, 0.0f }, .normal = {  0.0f,  0.0f, -1.0f } },
    { .position = { -0.5f,  0.5f, -0.5f }, .color = { 0.0f, 0.5f, 1.0f }, .uv = { 1.0f, 1.0f }, .normal = {  0.0f,  0.0f, -1.0f } },
    { .position = {  0.5f,  0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.5f }, .uv = { 0.0f, 1.0f }, .normal = {  0.0f,  0.0f, -1.0f } },

    { .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.7f, 0.2f, 0.2f }, .uv = { 0.0f, 0.0f }, .normal = { -1.0f,  0.0f,  0.0f } },
    { .position = { -0.5f, -0.5f,  0.5f }, .color = { 0.2f, 0.7f, 0.2f }, .uv = { 1.0f, 0.0f }, .normal = { -1.0f,  0.0f,  0.0f } },
    { .position = { -0.5f,  0.5f,  0.5f }, .color = { 0.2f, 0.2f, 0.7f }, .uv = { 1.0f, 1.0f }, .normal = { -1.0f,  0.0f,  0.0f } },
    { .position = { -0.5f,  0.5f, -0.5f }, .color = { 0.7f, 0.7f, 0.2f }, .uv = { 0.0f, 1.0f }, .normal = { -1.0f,  0.0f,  0.0f } },

    { .position = { -0.5f,  0.5f,  0.5f }, .color = { 0.7f, 0.2f, 0.7f }, .uv = { 0.0f, 0.0f }, .normal = {  0.0f,  1.0f,  0.0f } },
    { .position = {  0.5f,  0.5f,  0.5f }, .color = { 0.2f, 0.7f, 0.7f }, .uv = { 1.0f, 0.0f }, .normal = {  0.0f,  1.0f,  0.0f } },
    { .position = {  0.5f,  0.5f, -0.5f }, .color = { 0.8f, 0.8f, 0.8f }, .uv = { 1.0f, 1.0f }, .normal = {  0.0f,  1.0f,  0.0f } },
    { .position = { -0.5f,  0.5f, -0.5f }, .color = { 0.3f, 0.3f, 0.3f }, .uv = { 0.0f, 1.0f }, .normal = {  0.0f,  1.0f,  0.0f } },

    { .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.9f, 0.4f, 0.4f }, .uv = { 0.0f, 0.0f }, .normal = {  0.0f, -1.0f,  0.0f } },
    { .position = {  0.5f, -0.5f, -0.5f }, .color = { 0.4f, 0.9f, 0.4f }, .uv = { 1.0f, 0.0f }, .normal = {  0.0f, -1.0f,  0.0f } },
    { .position = {  0.5f, -0.5f,  0.5f }, .color = { 0.4f, 0.4f, 0.9f }, .uv = { 1.0f, 1.0f }, .normal = {  0.0f, -1.0f,  0.0f } },
    { .position = { -0.5f, -0.5f,  0.5f }, .color = { 0.9f, 0.9f, 0.4f }, .uv = { 0.0f, 1.0f }, .normal = {  0.0f, -1.0f,  0.0f } }
};

unsigned int cube_indices[] = {
    0, 1, 2,   0, 2, 3,
    4, 5, 6,   4, 6, 7,
    8, 9, 10,  8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23
};

size_t cube_vertex_count = sizeof(cube) / sizeof(cube[0]);
GLsizei cube_index_count = sizeof(cube_indices) / sizeof(cube_indices[0]);

LightColor sunlight = {
    .ambient = {{0.05f, 0.05f, 0.05f}},
    .diffuse = {{0.4f, 0.4f, 0.4f}},
    .specular = {{0.5f, 0.5f, 0.5f}}
};

LightColor point_light_colors[MAX_POINT_LIGHTS] = {
    {
        .ambient  = {{0.02f, 0.002f, 0.002f}},
        .diffuse  = {{1.0f,  0.1f,   0.1f}},
        .specular = {{1.0f,  0.3f,   0.3f}}
    },
    {
        .ambient  = {{0.002f, 0.02f, 0.002f}},
        .diffuse  = {{0.1f,   1.0f,  0.1f}},
        .specular = {{0.3f,   1.0f,  0.3f}}
    },
    {
        .ambient  = {{0.002f, 0.002f, 0.02f}},
        .diffuse  = {{0.1f,   0.1f,   1.0f}},
        .specular = {{0.3f,   0.3f,   1.0f}}
    },
    {
        .ambient  = {{0.02f, 0.0154f, 0.0112f}},
        .diffuse  = {{1.0f,  0.77f,   0.56f}},
        .specular = {{1.0f,  0.77f,   0.56f}}
    }
};

vec3s point_light_positions[] = {
    {{ 0.7f,  0.2f,   2.0f}},
    {{ 2.3f, -3.3f,  -4.0f}},
    {{-4.0f,  2.0f, -12.0f}},
    {{ 0.0f,  0.0f,  -3.0f}}
};

LightColor spotlight_color = {
    //255, 197, 143
    .ambient  = {{0.02f, 0.0154f, 0.0112f}},
    .diffuse  = {{1.0f,  0.77f,   0.56f}},
    .specular = {{1.0f,  0.77f,   0.56f}}
};

static void fps_counter(double *delta_time, double *title_countdown_time, GLFWwindow* window)
{
    *title_countdown_time -= *delta_time;
    if ( *title_countdown_time <= 0.0 && *delta_time > 0.0 ) {
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

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    vec2s offsets = input_get_mouse_offsets(xpos,ypos);
    handle_mouse(&renderer.camera, offsets, true);
} 

void draw_render_object(struct RenderObject *render_object, GLint model_location, float time, int i)
{
    vec3s rotation_axis = {1.0f, 0.3f, 0.5f};
    // vec3s scale_vec = {glm_rad((2.0f * (i + 1) * time)),glm_rad((2.0f * (i + 1) * time)),glm_rad(2.0f * (i + 1) * time)};
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_object->mesh.texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, render_object->mesh.texture2);
    glBindVertexArray(render_object->mesh.vao);
    identity_model(render_object);
    translate_model_matrix(render_object, render_object->position);
    rotate_model(render_object, glm_rad(20.0f * (i + 1) * time), rotation_axis);

    // scale_model(&renderer_state->render_objects.items[i],  scale_vec);
    glUniformMatrix4fv(model_location, 1,GL_FALSE, (float *)render_object->model);
    glDrawElements(GL_TRIANGLES, render_object->mesh.index_count, GL_UNSIGNED_INT, 0);    
}

static void run_render_loop(GLFWwindow* window, bool fps_enabled, struct RendererState *renderer_state)
{
    double previous_time = glfwGetTime();
    double title_countdown_time = 0.1;
    double delta_time = 0.0;
    bool render_state_updated = false;
    
    while(!glfwWindowShouldClose(window))
    {
        double current_time = glfwGetTime();
        // Update delta time and previous time for next frame
        update_frame_time(current_time, &previous_time, &delta_time);

        if(fps_enabled)
        {
            fps_counter(&delta_time, &title_countdown_time, window);
        }

        // Update window events
        glfwPollEvents();

        // Wipe drawing surface clear
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //TODO: Camera movement
        vec2s movement_axis = input_get_movement_axis();
        camera_movement(&renderer_state->camera, movement_axis, delta_time);
        camera_update(&renderer_state->camera);

        // Put the shader program and VAO in focus in OpenGL's state machine
        glUseProgram(renderer_state->shader_program);

        glUniform1f(renderer_state->material_shininess_location, 32.0f);

        glUniformMatrix4fv(
            renderer_state->view_location,
            1,
            GL_FALSE, 
            (float *)renderer_state->camera.view.raw
        );

        glUniform3fv(
            renderer_state->view_pos_location,
            1,
            renderer_state->camera.cameraPos.raw
        );

        glActiveTexture(GL_TEXTURE0);
        
        DirectionalLight *light = &renderer_state->directional_light;

        DirectionalLightUniforms *uniforms = &renderer_state->directional_light_uniforms;

        glUniform3fv(
            uniforms->direction,
            1,
            light->direction.raw
        );
        
        glUniform3fv(
            uniforms->ambient,
            1,
            light->color.ambient.raw
        );
        glUniform3fv(
            uniforms->diffuse,
            1,
            light->color.diffuse.raw
        );
        glUniform3fv(
            uniforms->specular,
            1,
            light->color.specular.raw
        );

        glUniform1i(renderer_state->point_light_count_location, (GLint)renderer_state->point_lights.count);

        for (size_t i = 0; i < renderer_state->point_lights.count; i++)
        {
            PointLight *light = &renderer_state->point_lights.items[i];

            PointLightUniforms *uniforms = &renderer_state->point_light_uniforms[i];

            glUniform3fv(
                uniforms->position,
                1,
                light->position.raw
            );
            glUniform3fv(
                uniforms->ambient,
                1,
                light->color.ambient.raw
            );
            glUniform3fv(
                uniforms->diffuse,
                1,
                light->color.diffuse.raw
            );
            glUniform3fv(
                uniforms->specular,
                1,
                light->color.specular.raw
            );
            glUniform1f(
                uniforms->constant,
                light->constant
            );
            glUniform1f(
                uniforms->linear,
                light->linear
            );
            glUniform1f(
                uniforms->quadratic,
                light->quadratic
            );
        }

        SpotLight *spot = &renderer_state->spot_light;
        SpotLightUniforms *spot_uniforms = &renderer_state->spot_light_uniforms;

        glUniform3fv(
            spot_uniforms->position,
            1,
            spot->position.raw
        );
        glUniform3fv(
            spot_uniforms->direction,
            1,
            spot->direction.raw
        );
        glUniform3fv(
            spot_uniforms->ambient,
            1,
            spot->color.ambient.raw
        );
        glUniform3fv(
            spot_uniforms->diffuse,
            1,
            spot->color.diffuse.raw
        );
        glUniform3fv(
            spot_uniforms->specular,
            1,
            spot->color.specular.raw
        );
        glUniform1f(
            spot_uniforms->constant,
            spot->constant
        );
        glUniform1f(
            spot_uniforms->linear,
            spot->linear
        );
        glUniform1f(
            spot_uniforms->quadratic,
            spot->quadratic
        );
        glUniform1f(
            spot_uniforms->inner_cutoff,
            cosf(glm_rad(spot->inner_cutoff_degrees))
        );
        glUniform1f(
            spot_uniforms->outer_cutoff,
            cosf(glm_rad(spot->outer_cutoff_degrees))
        );

        for(int i = 0; i < renderer_state->render_objects.count; i++)
        {
           draw_render_object(&renderer_state->render_objects.items[i], renderer_state->model_location, (float)glfwGetTime(), i);
        }

        // Put the drawing into the visible area
        glfwSwapBuffers(window);

        render_state_updated = false;
    }
}

static void configure_renderer_state()
{
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

static int renderer_init(struct RendererState *renderer)
{
    // configure_renderer_state();
    GLuint vs, fs;

    renderobject_array_initialize(&renderer->render_objects);
    for(int i = 0; i < 10; i++)
    { 
        RenderObject new_render_object = {0};
        
        int mesh_status = create_mesh_from_vertices(&new_render_object.mesh, cube, cube_vertex_count, cube_indices, cube_index_count);
        if(mesh_status != 0)
        {
            fprintf(stderr, "Mesh failed to be created, exiting!");
            return 1;
        }
        if(create_texture(&new_render_object.mesh, "assets/diffuse maps/container2.png") != 0){return 1;}
        if(create_texture(&new_render_object.mesh, "assets/specular maps/container2_specular.png") != 0){return 1;}
        renderobject_array_append(&renderer->render_objects, new_render_object);
    }

    RenderObject lamp_visual = {0};
    create_mesh_from_vertices(&lamp_visual.mesh, cube, cube_vertex_count, cube_indices, cube_index_count);
    lamp_visual.position = (vec3s){0.0f, 0.0f, 1.5f};
    lamp_visual.scale = (vec3s){{0.2f, 0.2f, 0.2f}};
    vec3s lamp_color = (vec3s){1.0f, 1.0f, 1.0f};

    if (load_shaders(&vs, &fs, "src/renderer/shaders/light.vert" ,"src/renderer/shaders/light.frag") != 0) {
        return 1;
    }

    if (create_shader_program(&vs, &fs, &renderer->shader_program) != 0) {
        return 1;
    }

    if (load_shaders(&vs, &fs, "src/renderer/shaders/lit.vert", "src/renderer/shaders/lit.frag") != 0)
    {
        return 1;
    }
    if (create_shader_program(&vs, &fs, &renderer->lamp_shader_program) != 0)
    {
        return 1;
    }

    renderer->lamp_model_location =
        glGetUniformLocation(renderer->lamp_shader_program, "model");
    renderer->lamp_view_location =
        glGetUniformLocation(renderer->lamp_shader_program, "view");
    renderer->lamp_projection_location =
        glGetUniformLocation(renderer->lamp_shader_program, "projection");
    renderer->lamp_light_color_location =
        glGetUniformLocation(renderer->lamp_shader_program, "lightColor");

    directional_light_init(&renderer->directional_light, (vec3s){{-0.2f, -1.0f, -0.3f}},sunlight);

    renderer->directional_light_uniforms.direction =
        glGetUniformLocation(renderer->shader_program, "directionalLight.direction");
    
    renderer->directional_light_uniforms.ambient =
        glGetUniformLocation(renderer->shader_program, "directionalLight.ambient");

    renderer->directional_light_uniforms.diffuse =
        glGetUniformLocation(renderer->shader_program, "directionalLight.diffuse");

    renderer->directional_light_uniforms.specular =
        glGetUniformLocation(renderer->shader_program, "directionalLight.specular");

    point_light_collection_init(&renderer->point_lights);

    for(size_t i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        PointLight light = {0};

        point_light_init(&light, point_light_positions[i], point_light_colors[i], 1.0f, 0.09f, 0.032f);

        point_light_collection_add(&renderer->point_lights, light);
    }

    // PointLightUniforms *point_uniforms = &renderer->point_light_uniforms;

    renderer->point_light_count_location = glGetUniformLocation(renderer->shader_program, "pointLightCount");

    for(size_t i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        PointLightUniforms *uniforms = &renderer->point_light_uniforms[i];

        char name[64];

        snprintf(name, sizeof(name), "pointLights[%zu].position", i);
        uniforms->position = glGetUniformLocation(renderer->shader_program, name);

        snprintf(name, sizeof(name), "pointLights[%zu].ambient", i);
        uniforms->ambient = glGetUniformLocation(renderer->shader_program, name);

        snprintf(name, sizeof(name), "pointLights[%zu].diffuse", i);
        uniforms->diffuse = glGetUniformLocation(renderer->shader_program, name);

        snprintf(name, sizeof(name), "pointLights[%zu].specular", i);
        uniforms->specular = glGetUniformLocation(renderer->shader_program, name);

        snprintf(name, sizeof(name), "pointLights[%zu].constant", i);
        uniforms->constant = glGetUniformLocation(renderer->shader_program, name);

        snprintf(name, sizeof(name), "pointLights[%zu].linear", i);
        uniforms->linear = glGetUniformLocation(renderer->shader_program, name);

        snprintf(name, sizeof(name), "pointLights[%zu].quadratic", i);
        uniforms->quadratic = glGetUniformLocation(renderer->shader_program, name);
    }

    spot_light_init(&renderer->spot_light, (vec3s){{0.0f, 15.0f, -7.5f}}, (vec3s){{0.0f, -1.0f, 0.0f}}, spotlight_color, 1.0f, 0.045f, 0.0075f, 30.0f, 40.0f);

    SpotLightUniforms *spot_uniforms = &renderer->spot_light_uniforms;

    spot_uniforms->position =
        glGetUniformLocation(renderer->shader_program, "spotLight.position");
    spot_uniforms->direction =
        glGetUniformLocation(renderer->shader_program, "spotLight.direction");
    spot_uniforms->ambient =
        glGetUniformLocation(renderer->shader_program, "spotLight.ambient");
    spot_uniforms->diffuse =
        glGetUniformLocation(renderer->shader_program, "spotLight.diffuse");
    spot_uniforms->specular =
        glGetUniformLocation(renderer->shader_program, "spotLight.specular");
    spot_uniforms->constant =
        glGetUniformLocation(renderer->shader_program, "spotLight.constant");
    spot_uniforms->linear =
        glGetUniformLocation(renderer->shader_program, "spotLight.linear");
    spot_uniforms->quadratic =
        glGetUniformLocation(renderer->shader_program, "spotLight.quadratic");
    spot_uniforms->inner_cutoff =
        glGetUniformLocation(renderer->shader_program, "spotLight.innerCutoff");
    spot_uniforms->outer_cutoff =
        glGetUniformLocation(renderer->shader_program, "spotLight.outerCutoff");
    
    camera_init(&renderer->camera);
    mat4s view_location = renderer->camera.view;

    renderer->projection_location = glGetUniformLocation(renderer->shader_program, "projection");
    glm_perspective(glm_rad(renderer->camera.cameraFOV), 800.0f/600.0f, 0.1f, 100.0f, renderer->projection);
    // glm_mat4_identity(renderer->camera.view.raw);
    // glm_translate(renderer->camera.view.raw, (vec3){0.0f, 0.0f, -3.0f});
    glUseProgram(renderer->shader_program);
    glUniformMatrix4fv(renderer->projection_location, 1, GL_FALSE, (float *)renderer->projection);
    renderer->view_location = glGetUniformLocation(renderer->shader_program, "view");
    glUseProgram(renderer->shader_program);
    glUniformMatrix4fv(renderer->view_location, 1, GL_FALSE, (float *)renderer->camera.view.raw);


    glEnable(GL_DEPTH_TEST);  
    renderer->model_location = glGetUniformLocation(renderer->shader_program, "model");
    glUniform1i(glGetUniformLocation(renderer->shader_program, "texture1"), 0);
    glUniform1i(glGetUniformLocation(renderer->shader_program, "texture2"), 1);
    if(renderer->model_location < 0)
    {
        fprintf(stderr, "Failed to get uniform location");
        return 1;
    }

    renderer->material_diffuse_location =
        glGetUniformLocation(renderer->shader_program, "material.diffuse");
    glUniform1i(renderer->material_diffuse_location, 0);
    renderer->material_specular_location =
        glGetUniformLocation(renderer->shader_program, "material.specular");
    glUniform1i(renderer->material_specular_location, 1);
    renderer->material_shininess_location =
        glGetUniformLocation(renderer->shader_program, "material.shininess");

    renderer->view_pos_location =
        glGetUniformLocation(renderer->shader_program, "viewPos");

    vec3s cubePositions[] = {
        (vec3s){{ 0.0f,  0.0f,  0.0f }},
        (vec3s){{ 2.0f,  5.0f, -15.0f }},
        (vec3s){{-1.5f, -2.2f, -2.5f }},
        (vec3s){{-3.8f, -2.0f, -12.3f }},
        (vec3s){{ 2.4f, -0.4f, -3.5f }},
        (vec3s){{-1.7f,  3.0f, -7.5f }},
        (vec3s){{ 1.3f, -2.0f, -2.5f }},
        (vec3s){{ 1.5f,  2.0f, -2.5f }},
        (vec3s){{ 1.5f,  0.2f, -1.5f }},
        (vec3s){{-1.3f,  1.0f, -1.5f }}
    };

    for(int i = 0; i < renderer->render_objects.count; i++)
    {
        renderer->render_objects.items[i].position = cubePositions[i];
        renderer->render_objects.items[i].rotation_angle = 0.0f;
        renderer->render_objects.items[i].scale = (vec3s){{1.0f, 1.0f, 1.0f}};
    }
    
    return 0;
}

static void renderer_shutdown(struct RendererState *renderer)
{
    for(int i = 0; i < renderer->render_objects.count; i++)
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

int renderer_run(GLFWwindow* window, bool fps_enabled)
{

    if (renderer_init(&renderer) != 0) {
        renderer_shutdown(&renderer);
        fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }
    run_render_loop(window, fps_enabled, &renderer);
    renderer_shutdown(&renderer);
    return 0;
}
