#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"

int renderer_init(GLFWwindow *window);
void renderer_render_frame(GLFWwindow *window, double delta_time);
void renderer_shutdown(void);
