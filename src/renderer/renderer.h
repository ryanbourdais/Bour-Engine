#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "camera.h"

int renderer_init(GLFWwindow *window, const Camera *camera);
void renderer_render_frame(GLFWwindow *window, const Camera *camera);
void renderer_shutdown(void);
