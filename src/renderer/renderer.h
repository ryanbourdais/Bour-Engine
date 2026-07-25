#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <stdbool.h>

int renderer_init(GLFWwindow *window);
int renderer_run(GLFWwindow *window, bool fps_enabled);
void renderer_render_frame(GLFWwindow *window, double delta_time);
void renderer_shutdown(void);
