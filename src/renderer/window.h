#pragma once
#include <stdbool.h>
#include <GLFW/glfw3.h>

GLFWwindow *window_create(bool fullscreen, bool vsync_enabled);
void window_poll_events(void);
bool window_should_close(GLFWwindow *window);
void window_get_framebuffer_size(GLFWwindow *window, int *width, int *height);
void window_present(GLFWwindow *window);
void window_destroy(GLFWwindow *window);
