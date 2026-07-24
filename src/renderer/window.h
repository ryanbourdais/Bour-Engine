#pragma once
#include <stdbool.h>
#include <GLFW/glfw3.h>

GLFWwindow *window_create(bool fullscreen);
void window_poll_events(void);
bool window_should_close(GLFWwindow *window);
void window_present(GLFWwindow *window);
void window_destroy(GLFWwindow *window);
