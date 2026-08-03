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

    const EditorHierarchyItem *hierarchy_items;
    size_t hierarchy_item_count;
} EditorFrameData;

typedef struct EditorFrameResult {
    unsigned int selected_entity_id;
    bool selection_changed;
} EditorFrameResult;

int editor_ui_init(struct GLFWwindow *window);
EditorFrameResult editor_ui_begin_frame(const EditorFrameData *frame);
void editor_ui_render(void);
void editor_ui_shutdown(void);

#ifdef __cplusplus
}
#endif