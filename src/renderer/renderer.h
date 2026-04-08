#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <stdbool.h>

int renderer_run(GLFWwindow* window, bool fps_enabled);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);