#pragma once

struct GLFWwindow;

#ifdef __cplusplus
extern "C"{
#endif

int editor_ui_init(struct GLFWwindow *window);
void editor_ui_begin_frame(void);
void editor_ui_render(void);
void editor_ui_shutdown(void);

#ifdef __cplusplus
}
#endif