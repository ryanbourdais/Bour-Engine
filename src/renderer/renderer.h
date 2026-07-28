#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "camera.h"

typedef struct RendererState Renderer;
typedef struct RendererFrame {
    const Camera *camera;
    int framebuffer_width;
    int framebuffer_height;
} RendererFrame;

Renderer *renderer_create(void);
int renderer_init(Renderer *renderer, GLFWwindow *window, const Camera *camera);
void renderer_render_frame(Renderer *renderer, const RendererFrame *frame);
void renderer_shutdown(Renderer *renderer);
void renderer_destroy(Renderer *renderer);
