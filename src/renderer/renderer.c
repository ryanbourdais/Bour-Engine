#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/struct.h>
#include "shaders.h"
#include "data_types/mesh.h"
#include "../controller/input.h"
#include "data_types/texture.h"
#include "data_types/renderObject.h"
#include "camera.h"


struct RendererState {
    GLuint shader_program;
    RenderObjectArray render_objects;
    GLint model_location;
    GLint projection_location;
    mat4 projection;
    mat4 view;
    GLint view_location;
    Camera camera;
};

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
    { .position = { -0.5f, -0.5f,  0.5f }, .color = { 1.0f, 0.0f, 0.0f }, .uv = { 0.0f, 0.0f } },
    { .position = {  0.5f, -0.5f,  0.5f }, .color = { 0.0f, 1.0f, 0.0f }, .uv = { 1.0f, 0.0f } },
    { .position = {  0.5f,  0.5f,  0.5f }, .color = { 0.0f, 0.0f, 1.0f }, .uv = { 1.0f, 1.0f } },
    { .position = { -0.5f,  0.5f,  0.5f }, .color = { 1.0f, 1.0f, 0.0f }, .uv = { 0.0f, 1.0f } },

    { .position = {  0.5f, -0.5f,  0.5f }, .color = { 0.0f, 1.0f, 1.0f }, .uv = { 0.0f, 0.0f } },
    { .position = {  0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 1.0f }, .uv = { 1.0f, 0.0f } },
    { .position = {  0.5f,  0.5f, -0.5f }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 1.0f, 1.0f } },
    { .position = {  0.5f,  0.5f,  0.5f }, .color = { 0.2f, 0.2f, 0.2f }, .uv = { 0.0f, 1.0f } },

    { .position = {  0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.5f, 0.0f }, .uv = { 0.0f, 0.0f } },
    { .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.5f, 1.0f, 0.0f }, .uv = { 1.0f, 0.0f } },
    { .position = { -0.5f,  0.5f, -0.5f }, .color = { 0.0f, 0.5f, 1.0f }, .uv = { 1.0f, 1.0f } },
    { .position = {  0.5f,  0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.5f }, .uv = { 0.0f, 1.0f } },

    { .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.7f, 0.2f, 0.2f }, .uv = { 0.0f, 0.0f } },
    { .position = { -0.5f, -0.5f,  0.5f }, .color = { 0.2f, 0.7f, 0.2f }, .uv = { 1.0f, 0.0f } },
    { .position = { -0.5f,  0.5f,  0.5f }, .color = { 0.2f, 0.2f, 0.7f }, .uv = { 1.0f, 1.0f } },
    { .position = { -0.5f,  0.5f, -0.5f }, .color = { 0.7f, 0.7f, 0.2f }, .uv = { 0.0f, 1.0f } },

    { .position = { -0.5f,  0.5f,  0.5f }, .color = { 0.7f, 0.2f, 0.7f }, .uv = { 0.0f, 0.0f } },
    { .position = {  0.5f,  0.5f,  0.5f }, .color = { 0.2f, 0.7f, 0.7f }, .uv = { 1.0f, 0.0f } },
    { .position = {  0.5f,  0.5f, -0.5f }, .color = { 0.8f, 0.8f, 0.8f }, .uv = { 1.0f, 1.0f } },
    { .position = { -0.5f,  0.5f, -0.5f }, .color = { 0.3f, 0.3f, 0.3f }, .uv = { 0.0f, 1.0f } },

    { .position = { -0.5f, -0.5f, -0.5f }, .color = { 0.9f, 0.4f, 0.4f }, .uv = { 0.0f, 0.0f } },
    { .position = {  0.5f, -0.5f, -0.5f }, .color = { 0.4f, 0.9f, 0.4f }, .uv = { 1.0f, 0.0f } },
    { .position = {  0.5f, -0.5f,  0.5f }, .color = { 0.4f, 0.4f, 0.9f }, .uv = { 1.0f, 1.0f } },
    { .position = { -0.5f, -0.5f,  0.5f }, .color = { 0.9f, 0.9f, 0.4f }, .uv = { 0.0f, 1.0f } }
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
        glClearColor( 0.6f, 0.6f, 0.8f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera_update(&renderer_state->camera);

        // Put the shader program and VAO in focus in OpenGL's state machine
        glUseProgram(renderer_state->shader_program);

        glUniformMatrix4fv(
            renderer_state->view_location,
            1,
            GL_FALSE,
            (float *)renderer_state->camera.view.raw
        );

        glActiveTexture(GL_TEXTURE0);
        
        for(int i = 0; i < renderer_state->render_objects.count; i++)
        {
            vec3s translation_vec = {1.0f, 0.3f, 0.5f};
            vec3s scale_vec = {glm_rad((2.0f * (i + 1) * (float)glfwGetTime())),glm_rad((2.0f * (i + 1) * (float)glfwGetTime())),glm_rad(2.0f * (i + 1) * (float)glfwGetTime())};
            glBindTexture(GL_TEXTURE_2D, renderer_state->render_objects.items[i].mesh.texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, renderer_state->render_objects.items[i].mesh.texture2);
            glBindVertexArray(renderer_state->render_objects.items[i].mesh.vao);
            identity_model(&renderer_state->render_objects.items[i]);
            translate_model_matrix(&renderer_state->render_objects.items[i], renderer_state->render_objects.items[i].position);
            rotate_model(&renderer_state->render_objects.items[i], glm_rad(20.0f * (i + 1) * (float)glfwGetTime()), translation_vec);
            // scale_model(&renderer_state->render_objects.items[i],  scale_vec);
            glUniformMatrix4fv(renderer_state->model_location, 1,GL_FALSE, (float *)renderer_state->render_objects.items[i].model);
            glDrawElements(GL_TRIANGLES, renderer_state->render_objects.items[i].mesh.index_count, GL_UNSIGNED_INT, 0);
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
        if(create_texture(&new_render_object.mesh, "assets/textures/container.png") != 0){return 1;}
        if(create_texture(&new_render_object.mesh, "assets/textures/awesomeface.png") != 0){return 1;}
        renderobject_array_append(&renderer->render_objects, new_render_object);
    }

    if (load_shaders(&vs, &fs) != 0) {
        return 1;
    }

    if (create_shader_program(&vs, &fs, &renderer->shader_program) != 0) {
        return 1;
    }

    camera_init(&renderer->camera);
    mat4s view_location = renderer->camera.view;

    renderer->projection_location = glGetUniformLocation(renderer->shader_program, "projection");
    glm_perspective(glm_rad(45.0f), 800.0f/600.0f, 0.1f, 100.0f, renderer->projection);
    glm_mat4_identity(renderer->camera.view.raw);
    glm_translate(renderer->camera.view.raw, (vec3){0.0f, 0.0f, -3.0f});
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
    struct RendererState renderer = {0};

    if (renderer_init(&renderer) != 0) {
        renderer_shutdown(&renderer);
        fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }
    run_render_loop(window, fps_enabled, &renderer);
    renderer_shutdown(&renderer);
    return 0;
}
