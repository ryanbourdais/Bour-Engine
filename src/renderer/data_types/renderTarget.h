#pragma once

#include <glad/glad.h>

typedef struct RenderTarget
{
    GLuint framebuffer;
    GLuint color_texture;
    GLuint depth_stencil_renderbuffer;

    int width;
    int height;
} RenderTarget;

typedef struct MsaaRenderTarget {
    GLuint framebuffer;
    GLuint color_renderbuffer;
    GLuint depth_stencil_renderbuffer;

    int width;
    int height;
    int samples;
} MsaaRenderTarget;


int render_target_init(RenderTarget *target, int width, int height);
void render_target_bind(RenderTarget *target);
void render_target_unbind(void);
void render_target_free(RenderTarget *target);
int msaa_render_target_init(MsaaRenderTarget *target, int width, int height, int samples);
void msaa_render_target_bind(MsaaRenderTarget *target);
void msaa_render_target_resolve_to(MsaaRenderTarget *source, RenderTarget *destination);
void msaa_render_target_free(MsaaRenderTarget *target);