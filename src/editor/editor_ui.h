#pragma once

#include <stddef.h>
#include <stdbool.h>

struct GLFWwindow;

#ifdef __cplusplus
extern "C"{
#endif

typedef struct EditorHierarchyItem {
    unsigned int entity_id;
    const char *name;
} EditorHierarchyItem;

typedef enum EditorSelectedLightType {
    EDITOR_SELECTED_LIGHT_NONE = 0,
    EDITOR_SELECTED_LIGHT_DIRECTIONAL,
    EDITOR_SELECTED_LIGHT_POINT,
    EDITOR_SELECTED_LIGHT_SPOT,
} EditorSelectedLightType;

typedef struct EditorFrameData {
    double delta_time;
    double fps;
    size_t entity_count;
    size_t renderable_count;
    unsigned int selected_entity_id;
    bool has_selected_entity;
    bool selected_entity_has_transform;
    const char *selected_entity_name;
    float selected_position[3];
    float selected_rotation[3];
    float selected_scale[3];
    bool editor_cursor_enabled;

    EditorSelectedLightType selected_light_type;
    float selected_light_ambient[3];
    float selected_light_diffuse[3];
    float selected_light_specular[3];
    float selected_light_direction[3];
    float selected_light_position[3];

    double profile_engine_update_ms;
    double profile_scene_extract_ms;
    double profile_editor_begin_ms;
    double profile_renderer_ms;
    double profile_editor_render_ms;
    double profile_present_ms;

    const EditorHierarchyItem *hierarchy_items;
    size_t hierarchy_item_count;
} EditorFrameData;

#define EDITOR_ENTITY_NAME_MAX_LENGTH 64
typedef struct EditorFrameResult {
    unsigned int selected_entity_id;
    bool selection_changed;
    bool toggle_editor_cursor;

    bool rename_selected_entity;
    char edited_name[EDITOR_ENTITY_NAME_MAX_LENGTH];

    bool create_empty_entity;
    bool create_renderable_entity;
    bool duplicate_selected_entity;

    bool light_changed;
    float edited_light_ambient[3];
    float edited_light_diffuse[3];
    float edited_light_specular[3];
    float edited_light_direction[3];
    float edited_light_position[3];

    bool transform_changed;
    float edited_position[3];
    float edited_rotation[3];
    float edited_scale[3];
} EditorFrameResult;

int editor_ui_init(struct GLFWwindow *window);
EditorFrameResult editor_ui_begin_frame(const EditorFrameData *frame);
void editor_ui_render(void);
void editor_ui_shutdown(void);

#ifdef __cplusplus
}
#endif