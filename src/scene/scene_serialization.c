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

static bool write_mesh_renderer_component(FILE *file, const MeshRendererComponent *mesh_renderer)
{
    if (file == NULL || mesh_renderer == NULL)
    {
        return false;
    }

    fprintf(file, "\"mesh_renderer\": {\n");
        
    write_indent(file, 4);
    fprintf(file, "\"source_type\": ");
    fprintf(file, "\"asset\"");
    fprintf(file, ",\n");

    write_indent(file, 4);
    fprintf(file, "\"model_path\": ");
    fprintf(file, "\"%s\"", mesh_renderer->model_path);
    fprintf(file, ",\n");

    write_indent(file, 4);
    fprintf(file, "\"material\": ");
    fprintf(file, "null");
    fprintf(file, ",\n");

    write_indent(file, 4);
    fprintf(file, "\"lod\": ");
    fprintf(file, "null\n");

    write_indent(file, 3);
    fprintf(file, "}");

    return true;
}

static bool write_camera_component(FILE *file, const CameraComponent *camera)
{
    if (file == NULL || camera == NULL)
    {
        return false;
    }

    fprintf(file, "\"camera\": {\n");
        
    write_indent(file, 4);
    fprintf(file, "\"fov\": ");
    fprintf(file, "%.3f", camera->fov);
    fprintf(file, ",\n");

    write_indent(file, 4);
    fprintf(file, "\"near_clip\": ");
    fprintf(file, "%.3f", camera->near_clip);
    fprintf(file, ",\n");

    write_indent(file, 4);
    fprintf(file, "\"far_clip\": ");
    fprintf(file, "%.3f", camera->far_clip);
    fprintf(file, "\n");

    write_indent(file, 3);
    fprintf(file, "}");

    return true;
}

static bool write_skybox_component(FILE *file, const SkyboxComponent *skybox)
{
    if (file == NULL || skybox == NULL)
    {
        return false;
    }

    fprintf(file, "\"skybox\": {\n");
        
    write_indent(file, 4);
    fprintf(file, "\"faces\": [\n");
    
    for (size_t i = 0; i < 6; i++)
    {
        write_indent(file, 5);
        fprintf(file, "\"%s\"", skybox->faces[i]);

        if (i + 1 < 6)
        {
            fprintf(file, ",");
        }

        fprintf(file, "\n");
    }

    write_indent(file, 4);
    fprintf(file, "]\n");

    write_indent(file, 3);
    fprintf(file, "}");

    return true;
}



static bool write_light_color_fields(FILE *file, const LightColor *color)
{
    if (file == NULL || color == NULL)
    {
        return false;
    }

    write_indent(file, 4);
    fprintf(file, "\"ambient\": ");
    write_vec3(file, &color->ambient);
    fprintf(file, ",\n");
    write_indent(file, 4);
    fprintf(file, "\"diffuse\": ");
    write_vec3(file, &color->diffuse);
    fprintf(file, ",\n");
    write_indent(file, 4);
    fprintf(file, "\"specular\": ");
    write_vec3(file, &color->specular);
    
    return true;
}

static bool write_directional_light_component(FILE *file, const DirectionalLightComponent *directional_light)
{
    if (file == NULL || directional_light == NULL)
    {
        return false;
    }

    fprintf(file, "\"directional_light\": {\n");
        
    write_indent(file, 4);
    fprintf(file, "\"direction\": ");
    write_vec3(file, &directional_light->light.direction);
    fprintf(file, ",\n");
    write_light_color_fields(file, &directional_light->light.color);
    fprintf(file, "\n");
    write_indent(file, 3);
    fprintf(file, "}");

    return true;
}

static bool write_point_light_component(FILE *file, const PointLightComponent *point_light)
{
    if (file == NULL || point_light == NULL)
    {
        return false;
    }

    fprintf(file, "\"point_light\": {\n");
        
    write_indent(file, 4);
    fprintf(file, "\"position\": ");
    write_vec3(file, &point_light->light.position);
    fprintf(file, ",\n");
    write_light_color_fields(file, &point_light->light.color);
    fprintf(file, ",\n");
    write_indent(file, 4);
    fprintf(file, "\"constant\": ");
    fprintf(file, "%.3f,\n", point_light->light.constant);
    write_indent(file, 4);
    fprintf(file, "\"linear\": ");
    fprintf(file, "%.3f,\n", point_light->light.linear);
    write_indent(file, 4);
    fprintf(file, "\"quadratic\": ");
    fprintf(file, "%.3f,\n", point_light->light.quadratic);
    write_indent(file, 4);
    fprintf(file, "\"has_visual\": ");
    fprintf(file, "%s\n", point_light->light.has_visual ? "true" : "false");
    write_indent(file, 3);
    fprintf(file, "}");

    return true;
}

static bool write_spot_light_component(FILE *file, const SpotLightComponent *spot_light)
{
    if (file == NULL || spot_light == NULL)
    {
        return false;
    }

    fprintf(file, "\"spot_light\": {\n");
        
    write_indent(file, 4);
    fprintf(file, "\"position\": ");
    write_vec3(file, &spot_light->light.position);
    fprintf(file, ",\n");
    write_indent(file, 4);
    fprintf(file, "\"direction\": ");
    write_vec3(file, &spot_light->light.direction);
    fprintf(file, ",\n");
    write_light_color_fields(file, &spot_light->light.color);
    fprintf(file, ",\n");
    write_indent(file, 4);
    fprintf(file, "\"constant\": ");
    fprintf(file, "%.3f,\n", spot_light->light.constant);
    write_indent(file, 4);
    fprintf(file, "\"linear\": ");
    fprintf(file, "%.3f,\n", spot_light->light.linear);
    write_indent(file, 4);
    fprintf(file, "\"quadratic\": ");
    fprintf(file, "%.3f,\n", spot_light->light.quadratic);
    write_indent(file, 4);
    fprintf(file, "\"inner_cutoff_degrees\": ");
    fprintf(file, "%.3f,\n", spot_light->light.inner_cutoff_degrees);
    write_indent(file, 4);
    fprintf(file, "\"outer_cutoff_degrees\": ");
    fprintf(file, "%.3f\n", spot_light->light.outer_cutoff_degrees);
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

    const MeshRendererComponent *mesh_renderer =
        component_storage_get_const(&scene->mesh_renderers, entity);

    const SkyboxComponent *skybox =
        component_storage_get_const(&scene->skyboxes, entity);

    const CameraComponent *camera =
        component_storage_get_const(&scene->cameras, entity);

    const DirectionalLightComponent *directional_light =
        component_storage_get_const(&scene->directional_lights, entity);
    
    const PointLightComponent *point_light =
        component_storage_get_const(&scene->point_lights, entity);

    const SpotLightComponent *spot_light =
        component_storage_get_const(&scene->spot_lights, entity);

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

    if (mesh_renderer != NULL)
    {
        if (fprintf(file, ",\n") < 0)
        {
            return false;
        }
        if (!write_indent(file, 3))
        {
            return false;
        }
        if (!write_mesh_renderer_component(file, mesh_renderer))
        {
            return false;
        }
    }

    if (skybox != NULL)
    {
        if (fprintf(file, ",\n") < 0)
        {
            return false;
        }
        if (!write_indent(file, 3))
        {
            return false;
        }
        if (!write_skybox_component(file, skybox))
        {
            return false;
        }
    }

    if (camera != NULL)
    {
        if (fprintf(file, ",\n") < 0)
        {
            return false;
        }
        if (!write_indent(file, 3))
        {
            return false;
        }
        if (!write_camera_component(file, camera))
        {
            return false;
        }
    }

    if (directional_light != NULL)
    {
        if (fprintf(file, ",\n") < 0)
        {
            return false;
        }
        if (!write_indent(file, 3))
        {
            return false;
        }
        if (!write_directional_light_component(file, directional_light))
        {
            return false;
        }
    }
    if (point_light != NULL)
    {
        if (fprintf(file, ",\n") < 0)
        {
            return false;
        }
        if (!write_indent(file, 3))
        {
            return false;
        }
        if (!write_point_light_component(file, point_light))
        {
            return false;
        }
    }
    if (spot_light != NULL)
    {
        if (fprintf(file, ",\n") < 0)
        {
            return false;
        }
        if (!write_indent(file, 3))
        {
            return false;
        }
        if (!write_spot_light_component(file, spot_light))
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
