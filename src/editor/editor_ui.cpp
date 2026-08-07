#include "editor_ui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>

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
        .toggle_editor_cursor = false,
        .rename_selected_entity = false,
        .edited_name = {0},
        .create_empty_entity = false,
        .create_renderable_entity = false,
        .duplicate_selected_entity = false,
        .transform_changed = false,
        .edited_position = {0.0f, 0.0f, 0.0f},
        .edited_rotation = {0.0f, 0.0f, 0.0f},
        .edited_scale = {0.0f, 0.0f, 0.0f},
    };

    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(260, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("Hierarchy");

    if (frame != nullptr)
    {
        for (size_t i = 0; i < frame->hierarchy_item_count; i++)
        {
            const EditorHierarchyItem *item = &frame->hierarchy_items[i];
            
            ImGui::PushID((int)item->entity_id);
            
            bool selected = item->entity_id == frame->selected_entity_id;
            if (ImGui::Selectable(item->name, selected))
            {
                result.selected_entity_id = item->entity_id;
                result.selection_changed = true;
            }
            ImGui::PopID();
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
        char name_buffer[EDITOR_ENTITY_NAME_MAX_LENGTH] = {0};
        snprintf(name_buffer, sizeof(name_buffer), "%s", frame->selected_entity_name);

        if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer)))
        {
            result.rename_selected_entity = true;
            snprintf(result.edited_name, sizeof(result.edited_name), "%s", name_buffer);
        }

        ImGui::Text("ID: %u", frame->selected_entity_id);

        if (frame->selected_entity_has_transform)
        {
            float position[3] = {
                frame->selected_position[0],
                frame->selected_position[1],
                frame->selected_position[2],
            };

            float rotation[3] = {
                frame->selected_rotation[0],
                frame->selected_rotation[1],
                frame->selected_rotation[2],
            };

            float scale[3] = {
                frame->selected_scale[0],
                frame->selected_scale[1],
                frame->selected_scale[2],
            };
            bool changed = false;

            changed |= ImGui::DragFloat3("Position", position, 0.05f);
            changed |= ImGui::DragFloat3("Rotation", rotation, 0.5f);
            changed |= ImGui::DragFloat3("Scale", scale, 0.05f);

            if (changed)
            {
                result.transform_changed = true;

                result.edited_position[0] = position[0];
                result.edited_position[1] = position[1];
                result.edited_position[2] = position[2];

                result.edited_rotation[0] = rotation[0];
                result.edited_rotation[1] = rotation[1];
                result.edited_rotation[2] = rotation[2];

                result.edited_scale[0] = scale[0];
                result.edited_scale[1] = scale[1];
                result.edited_scale[2] = scale[2];
            }
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

    ImGui::Separator();
    ImGui::Text("Engine Update: %.3f ms", frame->profile_engine_update_ms);
    ImGui::Text("Scene Extract: %.3f ms", frame->profile_scene_extract_ms);
    ImGui::Text("Editor Begin: %.3f ms", frame->profile_editor_begin_ms);
    ImGui::Text("Renderer: %.3f ms", frame->profile_renderer_ms);
    ImGui::Text("Editor Render: %.3f ms", frame->profile_editor_render_ms);
    ImGui::Text("Present: %.3f ms", frame->profile_present_ms);

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(280, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(220, 120), ImGuiCond_FirstUseEver);
    ImGui::Begin("Camera Settings");

    if (frame != nullptr)
    {
        ImGui::Text("Mouse Mode: %s", frame->editor_cursor_enabled ? "Editor" : "Camera");
        if (ImGui::Button(frame->editor_cursor_enabled ? "Switch to Camera Mouse" : "Switch to Editor Mouse"))
        {
            result.toggle_editor_cursor = true;
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(520, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(240, 140), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene Actions");

    if (ImGui::Button("Create Empty Entity"))
    {
        result.create_empty_entity = true;
    }
    if (ImGui::Button("Create Renderable Entity"))
    {
        result.create_renderable_entity = true;
    }
    if (frame != nullptr && frame->has_selected_entity)
    {
        if (ImGui::Button("Duplicate Selected"))
        {
            result.duplicate_selected_entity = true;
        }
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