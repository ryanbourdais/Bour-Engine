#include "renderTarget.h"

#include <stdio.h>

int render_target_init(RenderTarget *target, int width, int height)
{
    target->width = width;
    target->height = height;

    glGenFramebuffers(1, &target->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);

    glGenTextures(1, &target->color_texture);
    glBindTexture(GL_TEXTURE_2D, target->color_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

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
//TODO: render_target_resize

int msaa_render_target_init(MsaaRenderTarget *target, int width, int height, int samples)
{
    target->width = width;
    target->height = height;
    target->samples = samples;

    glGenFramebuffers(1, &target->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target->framebuffer);

    glGenRenderbuffers(1, &target->color_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, target->color_renderbuffer);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGB8, width, height);

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
