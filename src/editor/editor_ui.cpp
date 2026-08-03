#include "editor_ui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

int editor_ui_init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    return 0;
}

EditorFrameResult editor_ui_begin_frame(const EditorFrameData *frame)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    EditorFrameResult result = {
        .selected_entity_id = frame != nullptr ? frame->selected_entity_id : 0,
        .selection_changed = false,
    };

    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(260, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("Hierarchy");

    if (frame != nullptr)
    {
        for (size_t i = 0; i < frame->hierarchy_item_count; i++)
        {
            const EditorHierarchyItem *item = &frame->hierarchy_items[i];
            bool selected = item->entity_id == frame->selected_entity_id;
            if (ImGui::Selectable(item->name, selected))
            {
                result.selected_entity_id = item->entity_id;
                result.selection_changed = true;
            }
        }
    }

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(280, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_FirstUseEver);
    ImGui::Begin("Inspector");
    if (frame == nullptr || !frame->has_selected_entity)
    {
        ImGui::Text("No entity selected");
    }
    else {
        ImGui::Text("Entity: %s", frame->selected_entity_name);
        ImGui::Text("ID: %u", frame->selected_entity_id);

        if (frame->selected_entity_has_transform)
        {
            ImGui::Text("Position: %.2f, %.2f, %.2f",
                 frame->selected_position[0],
                 frame->selected_position[1],
                 frame->selected_position[2]
                );
            ImGui::Text("Rotation: %.2f, %.2f, %.2f",
                 frame->selected_rotation[0],
                 frame->selected_rotation[1],
                 frame->selected_rotation[2]
                );
            ImGui::Text("Scale: %.2f, %.2f, %.2f",
                 frame->selected_scale[0],
                 frame->selected_scale[1],
                 frame->selected_scale[2]
                );
        }
        else {
            ImGui::Text("No transform component");
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(10, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(220, 120), ImGuiCond_FirstUseEver);
    ImGui::Begin("Stats");
    if (frame != nullptr)
    {
        ImGui::Text("FPS: %.2f", frame->fps);
        ImGui::Text("Delta: %.3f ms", frame->delta_time * 1000.0);
        ImGui::Text("Entities: %zu", frame->entity_count);
        ImGui::Text("Renderables: %zu", frame->renderable_count);
    }

    ImGui::End();
    return result;
}

void editor_ui_render(void)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void editor_ui_shutdown(void)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}