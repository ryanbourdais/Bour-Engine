#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "camera.h"

typedef struct RendererState Renderer;

Renderer *renderer_create(void);
int renderer_init(Renderer *renderer, GLFWwindow *window, const Camera *camera);
void renderer_render_frame(Renderer *renderer, GLFWwindow *window, const Camera *camera);
void renderer_shutdown(Renderer *renderer);
void renderer_destroy(Renderer *renderer);