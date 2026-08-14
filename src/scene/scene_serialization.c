#include "scene_serialization.h"

#include <stdio.h>

static bool write_indent(FILE *file, int indent)
{
    if (file == NULL || indent < 0)
    {
        return false;
    }

    for(int i = 0; i < indent; i++)
    {
        if (fprintf(file, "  ") < 0)
        {
            return false;
        }
    }
    return true;
}

static bool write_vec3(FILE *file, const vec3s *value)
{
    if (file == NULL || value == NULL)
    {
        return false;
    }

    return fprintf(file, "[%.3f, %.3f, %.3f]", value->x, value->y, value->z) >= 0;
}

static bool write_name_component(FILE *file, const NameComponent *name)
{
    if (file == NULL || name == NULL)
    {
        return false;
    }

    if(name->value[0] == '\0')
    {
        
        return false;
    }

    return fprintf(file, "\"name\": \"%s\"", name->value) >= 0;
}

static bool write_transform_component(FILE *file, const TransformComponent *transform)
{
    if (file == NULL || transform == NULL)
    {
        return false;
    }

    fprintf(file, "\"transform\": {\n");
        
    write_indent(file, 4);
    fprintf(file, "\"position\": ");
    write_vec3(file, &transform->position);
    fprintf(file, ",\n");

    write_indent(file, 4);
    fprintf(file, "\"rotation\": ");
    write_vec3(file, &transform->rotation);
    fprintf(file, ",\n");

    write_indent(file, 4);
    fprintf(file, "\"scale\": ");
    write_vec3(file, &transform->scale);
    fprintf(file, "\n");

    write_indent(file, 3);
    fprintf(file, "}");

    return true;
}

static bool write_entity(FILE *file, const Scene *scene, EntityId entity)
{
    if (file == NULL || scene == NULL || entity == INVALID_ENTITY_ID)
    {
        return false;
    }

    const NameComponent *name =
        component_storage_get_const(&scene->names, entity);

    const TransformComponent *transform =
        component_storage_get_const(&scene->transforms, entity);

    if (!write_indent(file, 2))
    {
        return false;
    }

    if (fprintf(file, "{\n") < 0)
    {
        return false;
    }

    if (!write_indent(file, 3))
    {
        return false;
    }

    if (fprintf(file, "\"id\": %u", entity) < 0)
    {
        return false;
    }

    if (name != NULL && name->value[0] != '\0')
    {
        if (fprintf(file, ",\n") < 0)
        {
            return false;
        }

        if (!write_indent(file, 3))
        {
            return false;
        }

        if (!write_name_component(file, name))
        {
            return false;
        }
    }

    if (transform != NULL)
    {
        if (fprintf(file, ",\n") < 0)
        {
            return false;
        }
        if (!write_indent(file, 3))
        {
            return false;
        }
        if (!write_transform_component(file, transform))
        {
            return false;
        }
    }

    if (fprintf(file, "\n") < 0)
    {
        return false;
    }
    if (!write_indent(file, 2))
    {
        return false;
    }

    return fprintf(file, "}") >= 0;
}

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

    if (fprintf(file,
        "{\n"
        "  \"version\": 0,\n"
        "  \"active_camera\": {\n"
        "    \"entity\": %u\n"
        "  },\n"
        "  \"active_skybox\": {\n"
        "    \"entity\": %u\n"
        "  },\n"
        "  \"entities\": [\n",
        scene->active_camera,
        scene->active_skybox
    ) < 0)
    {
        fclose(file);
        return SCENE_SAVE_WRITE_FAILED;
    }

    for (size_t i = 0; i < scene->entities.count; i++)
    {
        EntityId entity = scene->entities.entities[i];

        if (!write_entity(file, scene, entity))
        {
            fclose(file);
            return SCENE_SAVE_WRITE_FAILED;
        }

        if (i + 1 < scene->entities.count)
        {
            if (fprintf(file, ",") < 0)
            {
                fclose(file);
                return SCENE_SAVE_WRITE_FAILED;
            }
        }

        if (fprintf(file, "\n") < 0)
        {
            fclose(file);
            return SCENE_SAVE_WRITE_FAILED;
        }
    }

    if (fprintf(file,
        "  ]\n"
        "}\n"
    ) < 0)
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
