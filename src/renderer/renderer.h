#pragma once
#include "glad/glad.h"


#include "camera.h"
#include "data_types/lightObject.h"
#include "renderer_data.h"

typedef struct RendererState Renderer;

typedef struct RendererViewport {
    int width;
    int height;
} RendererViewport;

typedef struct RendererFrame {
    const Camera *camera;
    RendererViewport viewport;

    const RenderableDrawData *renderables;
    size_t renderable_count;

    const DirectionalLight *directional_light;
    const PointLightCollection *point_lights;
    const SpotLightCollection *spot_lights;
} RendererFrame;

typedef struct RendererConfig {
    RendererViewport viewport;
    const Camera *camera;

    const char *model_path;
    const char *skybox_faces[6];
    const DirectionalLight *directional_light;
    const PointLightCollection *point_lights;
    const SpotLightCollection *spot_lights;
} RendererConfig;

typedef struct RendererStats {
    size_t mesh_count;
    size_t vertex_count;
    size_t triangle_count;
    size_t texture_count;
    size_t submitted_draw_count;
    size_t submitted_mesh_count;
    size_t submitted_vertex_count;
    size_t submitted_triangle_count;
    size_t missing_model_count;
} RendererStats;

Renderer *renderer_create(void);
int renderer_init(Renderer *renderer, const RendererConfig *config);
void renderer_render_frame(Renderer *renderer, const RendererFrame *frame);
void renderer_shutdown(Renderer *renderer);
void renderer_destroy(Renderer *renderer);
RendererStats renderer_get_stats(const Renderer *renderer);
RendererStats renderer_get_frame_stats(const Renderer *renderer, const RendererFrame *frame);
