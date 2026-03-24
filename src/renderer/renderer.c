#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include "shaders.h"
#include "data_types/mesh.h"
#include "../controller/input.h"


struct RendererState {
    GLuint shader_program;
    Mesh mesh;
    float_vec2 location;
    GLint mov_x_location;
    GLint mov_y_location;
};

Vertex triangle[] = {
    {
        .position = { 0.0f,  0.5f, 0.0f },
        .color    = { 1.0f,  0.0f, 0.0f }
    },
    {
        .position = { 0.5f, -0.5f, 0.0f },
        .color    = { 0.0f,  1.0f, 0.0f }
    },
    {
        .position = {-0.5f, -0.5f, 0.0f },
        .color    = { 0.0f,  0.0f, 1.0f }
    }
};

size_t vertex_count = sizeof(triangle) / sizeof(triangle[0]);


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

static void update_renderer_location(struct RendererState *renderer_state, double delta_time, bool *render_state_updated)
{
    float_vec2 movement_axis = input_get_movement_axis_smooth(5.0f, (float)delta_time);
    float movement_x = movement_axis.x;
    float movement_y = movement_axis.y;

    if(movement_x != 0 || movement_y != 0) {*render_state_updated = true;}
    
    renderer_state->location.x += movement_x;
    renderer_state->location.y += movement_y;
}

static void apply_renderer_uniforms(struct RendererState *renderer_state)
{
    glUniform1f(renderer_state->mov_x_location, renderer_state->location.x);
    glUniform1f(renderer_state->mov_y_location, renderer_state->location.y);
}

//TODO: Create MeshPool struct as storage for meshes then contain meshes there in rendererstate

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
        update_renderer_location(renderer_state, delta_time, &render_state_updated);

        // Update window events
        glfwPollEvents();

        // Wipe drawing surface clear
        glClearColor( 0.6f, 0.6f, 0.8f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Put the shader program and VAO in focus in OpenGL's state machine
        glUseProgram(renderer_state->shader_program);

        // Set shader uniforms
        if(render_state_updated) {apply_renderer_uniforms(renderer_state);}
        

        // glUniform1f(time_location, (float)current_time);
        glBindVertexArray(renderer_state->mesh.vao);

        // Draw points 0-3 from currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
    configure_renderer_state();
    GLuint vs, fs;

    int mesh_status = create_mesh_from_vertices(&renderer->mesh, triangle, vertex_count);
    if(mesh_status != 0)
    {
        fprintf(stderr, "Mesh failed to be created, exiting!");
        return 1;
    }

    if (load_shaders(&vs, &fs) != 0) {
        return 1;
    }

    if (create_shader_program(&vs, &fs, &renderer->shader_program) != 0) {
        return 1;
    }
    
    renderer->mov_x_location = glGetUniformLocation(renderer->shader_program, "mov_x");
    renderer->mov_y_location = glGetUniformLocation(renderer->shader_program, "mov_y");

    if (renderer->mov_x_location < 0 || renderer->mov_y_location < 0) {
        fprintf(stderr, "Failed to get uniform locations\n");
        return 1;
    }

    return 0;
}

static void renderer_shutdown(struct RendererState *renderer)
{
    glDeleteBuffers(1, &renderer->mesh.position_vbo);
    glDeleteBuffers(1, &renderer->mesh.color_vbo);
    glDeleteVertexArrays(1, &renderer->mesh.vao);
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
