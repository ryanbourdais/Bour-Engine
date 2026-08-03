#pragma once

#include <stddef.h>

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

    const EditorHierarchyItem *hierarchy_items;
    size_t hierarchy_item_count;
} EditorFrameData;

int editor_ui_init(struct GLFWwindow *window);
void editor_ui_begin_frame(const EditorFrameData *frame);
void editor_ui_render(void);
void editor_ui_shutdown(void);

#ifdef __cplusplus
}
#endif