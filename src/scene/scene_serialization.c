#include "scene_serialization.h"

#include <stdio.h>

SceneSaveResult scene_save_to_file(const Scene *scene, const char *path)
{
    if (scene == NULL || path == NULL)
    {
        return SCENE_SAVE_INVALID_ARGUMENT;
    }

    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        return SCENE_SAVE_FILE_OPEN_FAILED;
    }

    int written = fprintf(file,
        "{\n"
        "  \"version\": 0,\n"
        "  \"active_camera\": {\n"
        "    \"entity\": %u\n"
        "  },\n"
        "  \"active_skybox\": {\n"
        "    \"entity\": %u\n"
        "  },\n"
        "  \"entities\": []\n"
        "}\n",
        scene->active_camera,
        scene->active_skybox
    );

    if (written < 0)
    {
        fclose(file);
        return SCENE_SAVE_WRITE_FAILED;
    }

    if (fclose(file) != 0)
    {
        return SCENE_SAVE_WRITE_FAILED;
    }

    return SCENE_SAVE_OK;
}
