#pragma once

#include <stdint.h>
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

typedef struct EntityIdRenderTarget {
    GLuint framebuffer;
    GLuint entity_id_texture;
    GLuint depth_stencil_renderbuffer;

    int width;
    int height;
} EntityIdRenderTarget;

int render_target_init(RenderTarget *target, int width, int height);
void render_target_bind(RenderTarget *target);
void render_target_unbind(void);
void render_target_free(RenderTarget *target);
int msaa_render_target_init(MsaaRenderTarget *target, int width, int height, int samples);
void msaa_render_target_bind(MsaaRenderTarget *target);
void msaa_render_target_resolve_to(MsaaRenderTarget *source, RenderTarget *destination);
void msaa_render_target_free(MsaaRenderTarget *target);

int entity_id_render_target_init(
    EntityIdRenderTarget *target,
    int width,
    int height
);

void entity_id_render_target_bind(EntityIdRenderTarget *target);
void entity_id_render_target_free(EntityIdRenderTarget *target);

int entity_id_render_target_resize(
    EntityIdRenderTarget *target,
    int width,
    int height
);

uint32_t entity_id_render_target_read(
    const EntityIdRenderTarget *target,
    float u,
    float v
);

int render_target_resize(RenderTarget *target, int width, int height);
int msaa_render_target_resize(
    MsaaRenderTarget *target,
    int width,
    int height
);
