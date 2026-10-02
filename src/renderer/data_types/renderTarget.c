#include "renderTarget.h"

#include <stdint.h>
#include <stdio.h>

static void entity_id_render_target_allocate_storage(
    EntityIdRenderTarget *target,
    int width,
    int height
)
{
    glBindTexture(GL_TEXTURE_2D, target->entity_id_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R32UI,
        width,
        height,
        0,
        GL_RED_INTEGER,
        GL_UNSIGNED_INT,
        NULL
    );

    glBindRenderbuffer(
        GL_RENDERBUFFER,
        target->depth_stencil_renderbuffer
    );

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        width,
        height
    );
}

int render_target_init(RenderTarget *target, int width, int height)
{
    target->width = width;
    target->height = height;

    glGenFramebuffers(1, &target->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);

    glGenTextures(1, &target->color_texture);
    glBindTexture(GL_TEXTURE_2D, target->color_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->color_texture, 0);

    glGenRenderbuffers(1, &target->depth_stencil_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, target->depth_stencil_renderbuffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target->depth_stencil_renderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "RenderTarget framebuffer is not complete\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

void render_target_bind(RenderTarget *target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
    glViewport(0, 0, target->width, target->height);
}

void render_target_unbind(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_target_free(RenderTarget *target)
{
    if (target->depth_stencil_renderbuffer)
    {
        glDeleteRenderbuffers(1, &target->depth_stencil_renderbuffer);
    }
    if (target->color_texture)
    {
        glDeleteTextures(1, &target->color_texture);
    }
    if (target->framebuffer)
    {
        glDeleteFramebuffers(1, &target->framebuffer);
    }

    target->framebuffer = 0;
    target->color_texture = 0;
    target->depth_stencil_renderbuffer = 0;
    target->width = 0;
    target->height = 0;
}

int render_target_resize(RenderTarget *target, int width, int height)
{
    if (target == NULL || width <= 0 || height <= 0)
    {
        return 1;
    }

    if (target->width == width && target->height == height)
    {
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);

    glBindTexture(GL_TEXTURE_2D, target->color_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_SRGB8,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        NULL
    );

    glBindRenderbuffer(GL_RENDERBUFFER, target->depth_stencil_renderbuffer);
    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        width,
        height
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "RenderTarget framebuffer is not complete after resize\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 1;
    }

    target->width = width;
    target->height = height;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

int msaa_render_target_init(MsaaRenderTarget *target, int width, int height, int samples)
{
    target->width = width;
    target->height = height;
    target->samples = samples;

    glGenFramebuffers(1, &target->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);

    glGenRenderbuffers(1, &target->color_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, target->color_renderbuffer);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_SRGB8, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, target->color_renderbuffer);

    glGenRenderbuffers(1, &target->depth_stencil_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, target->depth_stencil_renderbuffer);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target->depth_stencil_renderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "MSAA RenderTarget framebuffer is not complete\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

int msaa_render_target_resize(
    MsaaRenderTarget *target,
    int width,
    int height
)
{
    if (target == NULL || width <= 0 || height <= 0)
    {
        return 1;
    }

    if (target->width == width && target->height == height)
    {
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);

    glBindRenderbuffer(GL_RENDERBUFFER, target->color_renderbuffer);
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER,
        target->samples,
        GL_SRGB8,
        width,
        height
    );

    glBindRenderbuffer(GL_RENDERBUFFER, target->depth_stencil_renderbuffer);
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER,
        target->samples,
        GL_DEPTH24_STENCIL8,
        width,
        height
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "MSAA RenderTarget framebuffer is not complete after resize\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 1;
    }

    target->width = width;
    target->height = height;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

void msaa_render_target_bind(MsaaRenderTarget *target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
    glViewport(0, 0, target->width, target->height);
}

void msaa_render_target_resolve_to(MsaaRenderTarget *source, RenderTarget *destination)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source->framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->framebuffer);

    glBlitFramebuffer(0, 0, source->width, source->height, 0, 0, destination->width, destination->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void msaa_render_target_free(MsaaRenderTarget *target)
{
    if (target->depth_stencil_renderbuffer)
    {
        glDeleteRenderbuffers(1, &target->depth_stencil_renderbuffer);
    }

    if (target->color_renderbuffer)
    {
        glDeleteRenderbuffers(1, &target->color_renderbuffer);
    }

    if (target->framebuffer)
    {
        glDeleteFramebuffers(1, &target->framebuffer);
    }

    target->framebuffer = 0;
    target->color_renderbuffer = 0;
    target->depth_stencil_renderbuffer = 0;
    target->width = 0;
    target->height = 0;
    target->samples = 0;
}

int entity_id_render_target_init(
    EntityIdRenderTarget *target,
    int width,
    int height
)
{
    if (target == NULL || width <= 0 || height <= 0)
    {
        return 1;
    }

    *target = (EntityIdRenderTarget){0};

    glGenFramebuffers(1, &target->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);

    glGenTextures(1, &target->entity_id_texture);
    glBindTexture(GL_TEXTURE_2D, target->entity_id_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        target->entity_id_texture,
        0
    );

    glGenRenderbuffers(1, &target->depth_stencil_renderbuffer);

    entity_id_render_target_allocate_storage(target, width, height);

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        target->depth_stencil_renderbuffer
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Entity ID framebuffer is not complete\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        entity_id_render_target_free(target);
        return 1;
    }

    target->width = width;
    target->height = height;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

void entity_id_render_target_bind(EntityIdRenderTarget *target)
{
    if (target == NULL)
    {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
    glViewport(0, 0, target->width, target->height);
}

void entity_id_render_target_free(EntityIdRenderTarget *target)
{
    if (target == NULL)
    {
        return;
    }

    if (target->depth_stencil_renderbuffer != 0)
    {
        glDeleteRenderbuffers(1, &target->depth_stencil_renderbuffer);
    }

    if (target->entity_id_texture != 0)
    {
        glDeleteTextures(1, &target->entity_id_texture);
    }

    if (target->framebuffer != 0)
    {
        glDeleteFramebuffers(1, &target->framebuffer);
    }

    *target = (EntityIdRenderTarget){0};
}

int entity_id_render_target_resize(
    EntityIdRenderTarget *target,
    int width,
    int height
)
{
    if (target == NULL || width <= 0 || height <= 0)
    {
        return 1;
    }

    if (target->width == width && target->height == height)
    {
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);
    entity_id_render_target_allocate_storage(target, width, height);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Entity ID framebuffer is not complete after resize\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 1;
    }

    target->width = width;
    target->height = height;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

uint32_t entity_id_render_target_read(
    const EntityIdRenderTarget *target,
    float u,
    float v
)
{
    if (target == NULL ||
        target->framebuffer == 0 ||
        target->width <= 0 ||
        target->height <= 0 )
    {
        return 0;
    }

    if ( u < 0.0f)
    {
        u = 0.0f;
    }
    else if ( u > 1.0f)
    { 
        u = 1.0f;
    }

    if ( v < 0.0f)
    {
        v = 0.0f;
    }
    else if (v > 1.0f)
    {
        v = 1.0f;
    }

    int pixel_x = (int)(u * (float)target->width);
    int pixel_y = (int)((1.0f - v) * (float)target->height);

    if (pixel_x >= target->width)
    {
        pixel_x = target->width - 1;
    }

    if (pixel_y >= target->height)
    {
        pixel_y = target->height - 1;
    }

    uint32_t entity_id = 0;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, target->framebuffer);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(
        pixel_x,
        pixel_y,
        1,
        1,
        GL_RED_INTEGER,
        GL_UNSIGNED_INT,
        &entity_id
    );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return entity_id;
}



