#include "scene_serialization.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jsmn.h>

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

    if (name->value[0] == '\0')
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

static bool json_token_equals(const char *json, const jsmntok_t *token, const char *value)
{
    if (json == NULL || token == NULL || value == NULL)
    {
        return false;
    }

    if (token->type != JSMN_STRING)
    {
        return false;
    }

    size_t value_length = strlen(value);
    size_t token_length = (size_t)(token->end - token->start);

    return token_length == value_length &&
        strncmp(json + token->start, value, value_length) == 0;
}

static int json_skip_token(const jsmntok_t *tokens, int index)
{
    int next = index + 1;

    if (tokens[index].type == JSMN_OBJECT)
    {
        for (int i = 0; i < tokens[index].size * 2; i++)
        {
            next = json_skip_token(tokens, next);
        }
    }
    else if (tokens[index].type == JSMN_ARRAY)
    {
        for (int i = 0; i < tokens[index].size; i++)
        {
            next = json_skip_token(tokens, next);
        }
    }

    return next;
}

static bool json_token_to_int(const char *json, const jsmntok_t *token, int *out_value)
{
    if (json == NULL || token == NULL || out_value == NULL)
    {
        return false;
    }

    if (token->type != JSMN_PRIMITIVE)
    {
        return false;
    }

    char buffer[32];
    size_t length = (size_t)(token->end - token->start);
    if (length == 0 || length >= sizeof(buffer))
    {
        return false;
    }
    
    memcpy(buffer, json + token->start, length);
    buffer[length] = '\0';

    char *end = NULL;
    long value = strtol(buffer, &end, 10);
    if (end == buffer || *end != '\0')
    {
        return false;
    }

    *out_value = (int)value;
    return true;
}

static bool json_object_find_field(const char *json, const jsmntok_t *tokens, int object_index, const char *field_name, int *out_value_index)
{
    if (json == NULL || tokens == NULL || field_name == NULL || out_value_index == NULL)
    {
        return false;
    }

    if (tokens[object_index].type != JSMN_OBJECT)
    {
        return false;
    }

    int index = object_index + 1;

    for (int pair = 0; pair < tokens[object_index].size; pair++)
    {
        const jsmntok_t *key = &tokens[index];

        if (json_token_equals(json, key, field_name))
        {
            *out_value_index = index + 1;
            return true;
        }
 
        index = json_skip_token(tokens, index + 1);
    }

    return false;
}

static bool json_object_get_int_field(const char *json, const jsmntok_t *tokens, int object_index, const char *field_name, int *out_value)
{
    if (json == NULL || tokens == NULL || field_name == NULL || out_value == NULL)
    {
        return false;
    }

    const jsmntok_t *object = &tokens[object_index];

    if (object->type != JSMN_OBJECT)
    {
        return false;
    }

    int index = object_index + 1;

    for (int pair = 0; pair < object->size; pair++)
    {
        const jsmntok_t *key = &tokens[index];
        const jsmntok_t *value = &tokens[index + 1];

        if (json_token_equals(json, key, field_name))
        {
            return json_token_to_int(json, value, out_value);
        }

        index = json_skip_token(tokens, index + 1);
    }

    return false;

}

static bool json_object_get_positive_int_field(const char *json, const jsmntok_t *tokens, int object_index, const char *field_name, int *out_value)
{
    if (!json_object_get_int_field(json, tokens, object_index, field_name, out_value))
    {
        return false;
    }

    return *out_value > 0;
}


static void scene_parse_summary_init(SceneParseSummary *summary)
{
    summary->active_camera_entity = 0;
    summary->active_skybox_entity = 0;
    summary->entity_ids = NULL;
    summary->entity_count = 0;    
}

static void scene_parse_summary_shutdown(SceneParseSummary *summary)
{
    free(summary->entity_ids);
    summary->entity_ids = NULL;
    summary->entity_count = 0;   
    summary->active_camera_entity = 0;
    summary->active_skybox_entity = 0;
}

static bool scene_parse_summary_has_entity(const SceneParseSummary *summary, int entity_id)
{
    for (int i = 0; i < summary->entity_count; i++)
    {
        if (summary->entity_ids[i] == entity_id)
        {
        return true;
        }
    }

    return false;
}

static void parsed_scene_v0_init(ParsedSceneV0 *scene)
{
    scene->active_camera_entity = 0;
    scene->active_skybox_entity = 0;
    scene->entities = NULL;
    scene->entity_count = 0;
}

static void parsed_scene_v0_shutdown(ParsedSceneV0 *scene)
{
    free(scene->entities);
    scene->entities = NULL;
    scene->entity_count = 0;
    scene->active_camera_entity = 0;
    scene->active_skybox_entity = 0;
}

static bool json_token_copy_string(const char *json, const jsmntok_t *token, char *out_buffer, size_t out_buffer_size)
{
    if (json == NULL || token == NULL || out_buffer == NULL || out_buffer_size == 0)
    {
        return false;
    }

    if (token->type != JSMN_STRING)
    {
        return false;
    }

    size_t length = (size_t)(token->end - token->start);
    if (length >= out_buffer_size)
    {
        return false;
    }

    memcpy(out_buffer, json + token->start, length);
    out_buffer[length] = '\0';
    
    return true;
}

static bool json_token_to_float(const char *json, const jsmntok_t *token, float *out_value)
{
    if (json == NULL || token == NULL || out_value == NULL)
    {
        return false;
    }

    if (token->type != JSMN_PRIMITIVE)
    {
        return false;
    }

    char buffer[32];
    size_t length = (size_t)(token->end - token->start);
    if (length == 0 || length >= sizeof(buffer))
    {
        return false;
    }
    
    memcpy(buffer, json + token->start, length);
    buffer[length] = '\0';

    char *end = NULL;
    float value = strtof(buffer, &end);
    if (end == buffer || *end != '\0')
    {
        return false;
    }

    *out_value = value;
    return true;
}

static bool json_array_to_vec3(const char *json, const jsmntok_t *tokens, int array_index, vec3s *out_value)
{
    if (json == NULL || tokens == NULL || out_value == NULL)
    {
        return false;
    }

    const jsmntok_t *array = &tokens[array_index];

    if (array->type != JSMN_ARRAY || array->size != 3)
    {
        return false;
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    if (!json_token_to_float(json, &tokens[array_index + 1], &x))
    {
        return false;
    }

    if (!json_token_to_float(json, &tokens[array_index + 2], &y))
    {
        return false;
    }
    if (!json_token_to_float(json, &tokens[array_index + 3], &z))
    {
        return false;
    }

    *out_value = (vec3s){{ x, y, z}};
    return true;
}

static SceneLoadResult parse_transform_component_v0(const char *json, const jsmntok_t *tokens, int transform_index, TransformComponent *out_transform)
{
    if (json == NULL || tokens == NULL || out_transform == NULL)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    if (tokens[transform_index].type != JSMN_OBJECT)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    int position_index = -1;
    int rotation_index = -1;
    int scale_index = -1;

    if (!json_object_find_field(json, tokens, transform_index, "position", &position_index))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_object_find_field(json, tokens, transform_index, "rotation", &rotation_index))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_object_find_field(json, tokens, transform_index, "scale", &scale_index))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_array_to_vec3(json, tokens, position_index, &out_transform->position))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_array_to_vec3(json, tokens, rotation_index, &out_transform->rotation))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_array_to_vec3(json, tokens, scale_index, &out_transform->scale))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    return SCENE_LOAD_OK;
}

static SceneLoadResult parse_camera_component_v0(const char *json, const jsmntok_t *tokens, int camera_index, CameraComponent *out_camera)
{
    if (json == NULL || tokens == NULL || out_camera == NULL)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    if (tokens[camera_index].type != JSMN_OBJECT)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    int fov_index = -1;
    int far_clip_index = -1;
    int near_clip_index = -1;

    if (!json_object_find_field(json, tokens, camera_index, "fov", &fov_index))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_object_find_field(json, tokens, camera_index, "far_clip", &far_clip_index))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_object_find_field(json, tokens, camera_index, "near_clip", &near_clip_index))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_token_to_float(json, &tokens[fov_index], &out_camera->fov))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_token_to_float(json, &tokens[far_clip_index], &out_camera->far_clip))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!json_token_to_float(json, &tokens[near_clip_index], &out_camera->near_clip))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    return SCENE_LOAD_OK;
}

static bool parsed_scene_entity_has_camera(const ParsedSceneV0 *scene, int entity_id)
{
    if (scene == NULL)
    {
        return false;
    }

    for (int i = 0; i < scene->entity_count; i++)
    {
        if (scene->entities[i].id == entity_id)
        {
            return scene->entities[i].has_camera;
        }
    }

    return false;
}

static SceneLoadResult parse_entity_v0_shallow(const char *json, const jsmntok_t *tokens, int entity_index, ParsedEntityV0 *out_entity)
{
    if (json == NULL || tokens == NULL || out_entity == NULL)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    if (tokens[entity_index].type != JSMN_OBJECT)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    out_entity->id = 0;
    out_entity->has_name = false;
    out_entity->name[0] = '\0';
    out_entity->has_transform = false;
    transform_component_init(&out_entity->transform);
    out_entity->has_camera = false;
    out_entity->camera.fov = 45.0f;
    out_entity->camera.near_clip = 0.1f;
    out_entity->camera.far_clip = 100.0f;

    int index = entity_index + 1;

    for (int pair = 0; pair < tokens[entity_index].size; pair++)
    {
        const jsmntok_t *key = &tokens[index];
        const jsmntok_t *value = &tokens[index + 1];

        if (json_token_equals(json, key, "id"))
        {
            if (!json_token_to_int(json, value, &out_entity->id) || out_entity->id <= 0)
            {
                return SCENE_LOAD_INVALID_SCENE;
            }
        }
        else if (json_token_equals(json, key, "name"))
        {
            if (!json_token_copy_string(json, value, out_entity->name, sizeof(out_entity->name)))
            {
                return SCENE_LOAD_INVALID_SCENE;
            }

            out_entity->has_name = true;
        }
        else if (json_token_equals(json, key, "transform"))
        {
            SceneLoadResult result = parse_transform_component_v0(json, tokens, index + 1, &out_entity->transform);

            if (result != SCENE_LOAD_OK)
            {
                return result;
            }
            out_entity->has_transform = true;
        }
        else if (json_token_equals(json, key, "camera"))
        {
            SceneLoadResult result = parse_camera_component_v0(json, tokens, index + 1, &out_entity->camera);
            if (result != SCENE_LOAD_OK)
            {
                return result;
            }
            out_entity->has_camera = true;
        }
        index = json_skip_token(tokens, index + 1);
    }

    if (out_entity->id <= 0)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!out_entity->has_transform)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    return SCENE_LOAD_OK;
}

static SceneLoadResult parse_entities_v0_shallow(const char *json, const jsmntok_t *tokens, int entities_index, ParsedSceneV0 *out_scene)
{
    const jsmntok_t *entities = &tokens[entities_index];

    if (entities->type != JSMN_ARRAY)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    if (entities->size == 0)
    {
        out_scene->entities = NULL;
        out_scene->entity_count = 0;
        return SCENE_LOAD_OK;
    }

    out_scene->entities = malloc((size_t)entities->size * sizeof(ParsedEntityV0));
    if (out_scene->entities == NULL)
    {
        return SCENE_LOAD_FILE_READ_FAILED;
    }

    out_scene->entity_count = entities->size;

    int index = entities_index + 1;

    for (int i = 0; i < entities->size; i++)
    {
        SceneLoadResult result = parse_entity_v0_shallow(json, tokens, index, &out_scene->entities[i]);

        if (result != SCENE_LOAD_OK)
        {
            return result;
        }

        index = json_skip_token(tokens, index);
    }

    return SCENE_LOAD_OK;
}

static SceneLoadResult validate_scene_entities_shallow(const char *json, const jsmntok_t *tokens, int entities_index, SceneParseSummary *summary)
{
    const jsmntok_t *entities = &tokens[entities_index];

    if (entities->type != JSMN_ARRAY)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    if (entities->size > 0)
    {
        summary->entity_ids = malloc((size_t)entities->size * sizeof(int));
        if (summary->entity_ids == NULL)
        {
            return SCENE_LOAD_FILE_READ_FAILED;
        }
    }

    int index = entities_index + 1;

    for (int i = 0; i < entities->size; i++)
    {
        if (tokens[index].type != JSMN_OBJECT)
        {
            return SCENE_LOAD_INVALID_SCENE;
        }

        int entity_id = 0;
        if (!json_object_get_positive_int_field(json, tokens, index, "id", &entity_id))
        {
            return SCENE_LOAD_INVALID_SCENE;
        }

        for (int seen = 0; seen < summary->entity_count; seen++)
        {
            if (summary->entity_ids[seen] == entity_id)
            {
                return SCENE_LOAD_INVALID_SCENE;
            }
        }

        summary->entity_ids[summary->entity_count] = entity_id;
        summary->entity_count++;

        index = json_skip_token(tokens, index);
    }

    return SCENE_LOAD_OK;
}

static SceneLoadResult validate_scene_v0_top_level(const char *json, const jsmntok_t *tokens, int token_count, SceneParseSummary *summary)
{
    if (token_count <= 0 || tokens[0].type != JSMN_OBJECT)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    bool found_version = false;
    bool found_active_camera = false;
    bool found_active_skybox = false;
    bool found_entities = false;

    int index = 1;
    for (int pair = 0; pair < tokens[0].size; pair++)
    {
        const jsmntok_t *key = &tokens[index];
        const jsmntok_t *value = &tokens[index + 1];

        if (json_token_equals(json, key, "version"))
        {
            int version = -1;
            if (!json_token_to_int(json, value, &version))
            {
                return SCENE_LOAD_INVALID_SCENE;
            }
            if (version != 0)
            {
                return SCENE_LOAD_INVALID_VERSION;
            }
            found_version = true;
        }
        else if (json_token_equals(json, key, "active_camera"))
        {
            int entity = 0;

            if (!json_object_get_int_field(json, tokens, index + 1, "entity", &entity))
            {
                return SCENE_LOAD_INVALID_SCENE;
            }

            if (entity <= 0)
            {
                return SCENE_LOAD_INVALID_SCENE;
            }

            found_active_camera = true;
            summary->active_camera_entity = entity;
        }
        else if (json_token_equals(json, key, "active_skybox"))
        {
            int entity = 0;

            if (!json_object_get_int_field(json, tokens, index + 1, "entity", &entity))
            {
                return SCENE_LOAD_INVALID_SCENE;
            }

            if (entity <= 0)
            {
                return SCENE_LOAD_INVALID_SCENE;
            }
            summary->active_skybox_entity = entity;
            found_active_skybox = true;
        }
        else if (json_token_equals(json, key, "entities"))
        {
            SceneLoadResult entity_result = validate_scene_entities_shallow(json, tokens, index + 1, summary);

            if (entity_result != SCENE_LOAD_OK)
            {   
                return entity_result;
            }

            found_entities = true;
        }
        index = json_skip_token(tokens, index + 1);
    }

    if (!found_version || !found_active_camera || !found_active_skybox || !found_entities)
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    if (!scene_parse_summary_has_entity(summary, summary->active_camera_entity))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }
    if (!scene_parse_summary_has_entity(summary, summary->active_skybox_entity))
    {
        return SCENE_LOAD_INVALID_SCENE;
    }

    return SCENE_LOAD_OK;
}

static SceneLoadResult read_file_to_buffer(const char *path, char **out_buffer, size_t *out_size)
{
    if (path == NULL || out_buffer == NULL || out_size == NULL)
    {
        return SCENE_LOAD_INVALID_ARGUMENT;
    }

    *out_buffer = NULL;
    *out_size = 0;

    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        return SCENE_LOAD_FILE_OPEN_FAILED;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return SCENE_LOAD_FILE_READ_FAILED;
    }

    long file_size = ftell(file);
    if (file_size < 0)
    {
        fclose(file);
        return SCENE_LOAD_FILE_READ_FAILED;
    }

    if (fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return SCENE_LOAD_FILE_READ_FAILED;
    }

    char *buffer = malloc((size_t)file_size + 1);
    if (buffer == NULL)
    {
        fclose(file);
        return SCENE_LOAD_FILE_READ_FAILED;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)file_size, file);
    if (bytes_read != (size_t)file_size)
    {
        free(buffer);
        fclose(file);
        return SCENE_LOAD_FILE_READ_FAILED;
    }

    buffer[file_size] = '\0';

    if (fclose(file) != 0)
    {
        free(buffer);
        return SCENE_LOAD_FILE_READ_FAILED;
    }

    *out_buffer = buffer;
    *out_size = (size_t)file_size;

    return SCENE_LOAD_OK;
}

static SceneLoadResult parse_scene_json(const char *json, size_t json_size, SceneParseSummary *out_summary, ParsedSceneV0 *out_scene)
{
    if (json == NULL || json_size == 0 || out_summary == NULL || out_scene == NULL)
    {
        return SCENE_LOAD_PARSE_FAILED;
    }

    jsmn_parser parser;
    jsmn_init(&parser);

    int token_count = jsmn_parse(&parser, json, json_size, NULL, 0);
    if (token_count < 0)
    {
        return SCENE_LOAD_PARSE_FAILED;
    }

    jsmntok_t *tokens = malloc((size_t)token_count * sizeof(jsmntok_t));
    if (tokens == NULL)
    {
        return SCENE_LOAD_FILE_READ_FAILED;
    }

    jsmn_init(&parser);
    int parsed = jsmn_parse(&parser, json, json_size, tokens, (unsigned int)token_count);
    if (parsed < 0)
    {
        free(tokens);
        return SCENE_LOAD_PARSE_FAILED;
    }

    if (parsed == 0 || tokens[0].type != JSMN_OBJECT)
    {
        free(tokens);
        return SCENE_LOAD_INVALID_SCENE;
    }

    scene_parse_summary_init(out_summary);
    parsed_scene_v0_init(out_scene);

    SceneLoadResult validation_result = validate_scene_v0_top_level(json, tokens, parsed, out_summary);

    if (validation_result != SCENE_LOAD_OK)
    {
        parsed_scene_v0_shutdown(out_scene);
        scene_parse_summary_shutdown(out_summary);
        free(tokens);
        return validation_result;
    }

    out_scene->active_camera_entity = out_summary->active_camera_entity;
    out_scene->active_skybox_entity = out_summary->active_skybox_entity;

    int entities_index = -1;
    if (!json_object_find_field(json, tokens, 0, "entities", &entities_index))
    {
        parsed_scene_v0_shutdown(out_scene);
        scene_parse_summary_shutdown(out_summary);
        free(tokens);
        return SCENE_LOAD_INVALID_SCENE;
    }

    SceneLoadResult entities_parse_result = parse_entities_v0_shallow(json, tokens, entities_index, out_scene);

    if (entities_parse_result != SCENE_LOAD_OK)
    {
        parsed_scene_v0_shutdown(out_scene);
        scene_parse_summary_shutdown(out_summary);
        free(tokens);
        return entities_parse_result;
    }

    if (!parsed_scene_entity_has_camera(out_scene, out_scene->active_camera_entity))
    {
        parsed_scene_v0_shutdown(out_scene);
        scene_parse_summary_shutdown(out_summary);
        free(tokens);
        return SCENE_LOAD_INVALID_SCENE;
    }

    free(tokens);
    return validation_result;
}

SceneLoadResult scene_load_from_file(Scene *scene, const char *path)
{
    if (scene == NULL || path == NULL)
    {
        return SCENE_LOAD_INVALID_ARGUMENT;
    }

    char *json = NULL;
    size_t json_size = 0;
    SceneLoadResult read_result = read_file_to_buffer(path, &json, &json_size);
    if (read_result != SCENE_LOAD_OK)
    {
        return read_result;
    }

    SceneParseSummary summary;
    ParsedSceneV0 parsed_scene;
    SceneLoadResult parse_result = parse_scene_json(json, json_size, &summary, &parsed_scene);

    if (parse_result == SCENE_LOAD_OK)
    {
        parsed_scene_v0_shutdown(&parsed_scene);
        scene_parse_summary_shutdown(&summary);
    }

    free(json);

    return parse_result;
}
