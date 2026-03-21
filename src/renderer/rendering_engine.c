//openGL 4.1 Core
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "rendering_engine.h"
#include "../utils/helpers/file_reader.h"

//triangle vertices
float points[] = {
    0.0f, 0.5f, 0.0f, //top x,y,z
    0.5f, -0.5f, 0.0f, //bottom right x,y,z
    -0.5f, -0.5f, 0.0f //bottom left x,y,z
};

void safe_exit() {
    glfwTerminate();
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void initialize_glfw()
{
    glfwSetErrorCallback(error_callback);
}

void create_window_context(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);
}

void close_window(GLFWwindow* window, bool *windowClosed)
{
    *windowClosed = true;
    glfwDestroyWindow(window);
}

void start_glad()
{
    int version_glad = gladLoadGL();
    if (version_glad == 0)
    {
        fprintf(stderr, "Failed to initialize GLAD");
        exit(-1);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        printf("Escape key pressed, closing window\n");
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLuint create_vbo()
{
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
    return vbo;
}

GLuint create_vao(GLuint *vbo)
{
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    return vao;
}

void check_shader_compile_status(GLuint shader, const char* shader_name)
{
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        char *info_log = malloc((size_t)log_length);
        if(info_log != NULL)
        {
            glGetShaderInfoLog(shader, log_length, NULL, info_log);
            fprintf(stderr, "Error compiling shader %s: %s\n", shader_name, info_log);
            free(info_log);
        }
        else
        {
            fprintf(stderr, "Error compiling shader %s: Unable to allocate memory for info log\n", shader_name);
        }

        glDeleteShader(shader);
        exit(1);
    }
}

void load_shaders(GLuint *vs, GLuint *fs)
{
    
    struct ShaderBuffer vertex_shader_buffer = read_shader_file("src/renderer/shaders/test.vert");
    struct ShaderBuffer fragment_shader_buffer = read_shader_file("src/renderer/shaders/test.frag");
    if(vertex_shader_buffer.data == NULL || fragment_shader_buffer.data == NULL) {
        free_shader_buffer(&vertex_shader_buffer);
        free_shader_buffer(&fragment_shader_buffer);
        fprintf(stderr, "Failed to read shader files\n");
        exit(1);
    }
    GLint vertex_length = (GLint)vertex_shader_buffer.length;
    GLint fragment_length = (GLint)fragment_shader_buffer.length;
    const GLchar * const * vertex_shader_source = (const GLchar * const *)&vertex_shader_buffer.data;
    const GLchar * const * fragment_shader_source = (const GLchar * const *)&fragment_shader_buffer.data;
    
    *vs = glCreateShader(GL_VERTEX_SHADER);
    *fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(*vs, 1, vertex_shader_source, &vertex_length);
    glCompileShader(*vs);
    check_shader_compile_status(*vs, "Vertex Shader");

    
    glShaderSource(*fs, 1, fragment_shader_source, &fragment_length);
    glCompileShader(*fs);
    check_shader_compile_status(*fs, "Fragment Shader");
    free_shader_buffer(&vertex_shader_buffer);
    free_shader_buffer(&fragment_shader_buffer);
}

void create_shader_program(GLuint *vs, GLuint *fs, GLuint *shader_program)
{
    *shader_program = glCreateProgram();
    glAttachShader(*shader_program, *fs);
    glAttachShader(*shader_program, *vs);
    glLinkProgram(*shader_program);
}

void Draw(GLFWwindow* window, GLuint *shader_program, GLuint *vao, bool fps_enabled)
{

    double previous_time = glfwGetTime();
    double title_countdown_time = 0.1;
    while(!glfwWindowShouldClose(window))
    {
        if(fps_enabled)
        {
            double current_time = glfwGetTime();
            double elapsed_time = current_time - previous_time;
            previous_time = current_time;

            title_countdown_time -= elapsed_time;
            if ( title_countdown_time <= 0.0 && elapsed_time > 0.0 ) {
                double fps = 1.0 / elapsed_time;

                // Create a string and put the FPS as the window title.
                char title[256];
                sprintf(title, "FPS = %.2lf", fps);
                glfwSetWindowTitle(window, title);
                title_countdown_time = 0.1;
            }
        }
        // Update window events
        glfwPollEvents();

        // Wipe drawing surface clear
        glClearColor( 0.6f, 0.6f, 0.8f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Put the shader program and VAO in focus in OpenGL's state machine
        glUseProgram(*shader_program);
        glBindVertexArray(*vao);

        // Draw points 0-3 from currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Put the drawing into the visible area
        glfwSwapBuffers(window);
    }
}

void cleanup(GLuint *vbo, GLuint *vao, GLuint *shader_program)
{
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, vao);
    glDeleteProgram(*shader_program);
}

void render(GLFWwindow* window, int *win_h, int *win_w, bool fps_enabled)
{
    GLuint vbo = create_vbo();
    GLuint vao = create_vao(&vbo);
    GLuint vs, fs;
    load_shaders(&vs, &fs);
    GLuint shader_program;
    create_shader_program(&vs, &fs, &shader_program);
    Draw(window, &shader_program, &vao, fps_enabled);
    cleanup(&vbo, &vao, &shader_program);
}

double get_time()
{
    double time = glfwGetTime();
    return time;
}

void swap_buffers(GLFWwindow* window)
{
    //vsync
    glfwSwapInterval(1);
    glfwSwapBuffers(window);
}

void create_window(bool *windowClosed, bool fullscreen, bool fps_enabled)
{
    GLFWmonitor *mon = NULL;
    int win_w = 800, win_h = 600; // Our window dimensions, in pixels.

    if ( fullscreen ) {
        mon = glfwGetPrimaryMonitor();

        const GLFWvidmode* mode = glfwGetVideoMode( mon );

        // Hinting these properties lets us use "borderless full screen" mode.
        glfwWindowHint( GLFW_RED_BITS, mode->redBits );
        glfwWindowHint( GLFW_GREEN_BITS, mode->greenBits );
        glfwWindowHint( GLFW_BLUE_BITS, mode->blueBits );
        glfwWindowHint( GLFW_REFRESH_RATE, mode->refreshRate );

        win_w = mode->width;  // Use our 'desktop' resolution for window size
        win_h = mode->height; // to get a 'full screen borderless' window.
    }

    GLFWwindow* window = glfwCreateWindow(
        win_w,
        win_h,
        "Extended OpenGL Init",
        mon,
        NULL
    );
    if(!window)
    {
        fprintf(stderr, "Window failed to be created");
        exit(-1);
    }
    create_window_context(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    start_glad();
    render(window, &win_h, &win_w, fps_enabled);
    close_window(window, windowClosed);
}

void set_hints()
{
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    // MSAA 8x
    glfwWindowHint( GLFW_SAMPLES, 8 );
}

int rendering_engine_entry(bool fullscreen, bool fps_enabled) {
    initialize_glfw();
    bool windowClosed = false;
    if(!glfwInit())
    {
        fprintf(stderr, "GLFW init failed");
    }
    set_hints();
    while(!windowClosed)
    {
        create_window(&windowClosed, fullscreen, fps_enabled);
    }
    safe_exit();
    return 0;
}