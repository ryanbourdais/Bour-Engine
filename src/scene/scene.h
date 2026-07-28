#pragma once

typedef struct Scene {
    const char *model_path;
    const char *skybox_faces[6];
} Scene;

void scene_init_default(Scene *scene);
void scene_shutdown(Scene *scene);
