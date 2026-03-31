#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <cglm/struct.h>
#include "shaders.h"
#include "data_types/mesh.h"
#include "../controller/input.h"
#include "data_types/texture.h"
#include "data_types/renderObject.h"


struct RendererState {
    GLuint shader_program;
    // float_vec2 location;
    // GLint mov_x_location;
    // GLint mov_y_location;
    RenderObjectArray render_objects;
    GLint model_location;
};

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

// static void update_renderer_location(struct RendererState *renderer_state, double delta_time, bool *render_state_updated)
// {
//     float_vec2 movement_axis = input_get_movement_axis_smooth(5.0f, (float)delta_time);
//     float movement_x = movement_axis.x;
//     float movement_y = movement_axis.y;

//     if(movement_x != 0 || movement_y != 0) {*render_state_updated = true;}
    
//     renderer_state->position.x += movement_x;
//     renderer_state->position.y += movement_y;
// }

// static void apply_renderer_uniforms(struct RendererState *renderer_state)
// {
//     glUniform1f(renderer_state->mov_x_location, renderer_state->location.x);
//     glUniform1f(renderer_state->mov_y_location, renderer_state->location.y);
    
// }

static void update_model_matrix(struct RenderObject *render_object)
{
    glm_mat4_identity(render_object->model);
    glm_translate(render_object->model, render_object->position.raw);
    glm_rotate(render_object->model, (float)glfwGetTime(), (vec3){0.0f, 0.0f, 1.0f});
    glm_scale(render_object->model, render_object->scale.raw);
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

        // Update location based on input
        // update_renderer_location(renderer_state, delta_time, &render_state_updated);

        // Update window events
        glfwPollEvents();

        // Wipe drawing surface clear
        glClearColor( 0.6f, 0.6f, 0.8f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Put the shader program and VAO in focus in OpenGL's state machine
        glUseProgram(renderer_state->shader_program);
        glUniform1i(glGetUniformLocation(renderer_state->shader_program, "texture1"), 0);
        glUniform1i(glGetUniformLocation(renderer_state->shader_program, "texture2"), 1);
        glActiveTexture(GL_TEXTURE0);
       

        for(int i = 0; i < renderer_state->render_objects.count; i++)
        {
            glBindTexture(GL_TEXTURE_2D, renderer_state->render_objects.items[i].mesh.texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, renderer_state->render_objects.items[i].mesh.texture2);
            glBindVertexArray(renderer_state->render_objects.items[i].mesh.vao);
            update_model_matrix(&renderer_state->render_objects.items[i]);
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
        if(create_texture(&new_render_object.mesh, cube_vertex_count, "assets/textures/container.png") != 0){return 1;}
        if(create_texture(&new_render_object.mesh, cube_vertex_count, "assets/textures/awesomeface.png") != 0){return 1;}
        renderobject_array_append(&renderer->render_objects, new_render_object);
    }

    if (load_shaders(&vs, &fs) != 0) {
        return 1;
    }

    if (create_shader_program(&vs, &fs, &renderer->shader_program) != 0) {
        return 1;
    }

    renderer->model_location = glGetUniformLocation(renderer->shader_program, "model");
    if(renderer->model_location < 0)
    {
        fprintf(stderr, "Failed to get uniform location");
        return 1;
    }

            for(int i = 0; i < renderer->render_objects.count; i++)
        {
        renderer->render_objects.items[i].position = (vec3s){{0.0f, 0.0f, 0.0f}};
        renderer->render_objects.items[i].rotation_angle = 0.0f;
        renderer->render_objects.items[i].scale = (vec3s){{1.0f, 1.0f, 1.0f}};
        }

    // renderer->mov_x_location = glGetUniformLocation(renderer->shader_program, "mov_x");
    // renderer->mov_y_location = glGetUniformLocation(renderer->shader_program, "mov_y");

    // if (renderer->mov_x_location < 0 || renderer->mov_y_location < 0) {
    //     fprintf(stderr, "Failed to get uniform locations\n");
    //     return 1;
    // }

    return 0;
}

static void renderer_shutdown(struct RendererState *renderer)
{
    for(int i = 0; i < renderer->render_objects.count; i++)
    {
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.position_vbo);
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.color_vbo);
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.uv_vbo);
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.texture);
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.texture2);
        glDeleteVertexArrays(1, &renderer->render_objects.items[i].mesh.vao);
        glDeleteBuffers(1, &renderer->render_objects.items[i].mesh.ebo);
    }
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
