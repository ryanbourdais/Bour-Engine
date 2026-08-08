#include "editor_ui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "../utils/math_utils.h"

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

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 display_size = io.DisplaySize;

    const float margin = 10.0f;

    const ImVec2 hierarchy_size = ImVec2(260, 300);
    const ImVec2 stats_size = ImVec2(260, 180);
    const ImVec2 inspector_size = ImVec2(320, 300);
    const ImVec2 scene_actions_size = ImVec2(320, 140);
    const ImVec2 camera_settings_size = ImVec2(260, 100);
    
    const ImVec2 hierarchy_pos = ImVec2(margin, margin);
    const ImVec2 inspector_pos = ImVec2(display_size.x - margin, margin);
    const ImVec2 stats_pos = ImVec2(margin, margin + hierarchy_size.y + margin);
    const ImVec2 camera_settings_pos = ImVec2(display_size.x * 0.5f, display_size.y - margin);
    const ImVec2 scene_actions_pos = ImVec2(display_size.x - margin, margin + inspector_size.y + margin);

    EditorFrameResult result = {
        .selected_entity_id = frame != nullptr ? frame->selected_entity_id : 0,
        .selection_changed = false,
        .toggle_editor_cursor = false,
        .rename_selected_entity = false,
        .edited_name = {0},
        .create_empty_entity = false,
        .create_renderable_entity = false,
        .duplicate_selected_entity = false,
        .delete_selected_entity = false,
        .transform_changed = false,
        .edited_position = {0.0f, 0.0f, 0.0f},
        .edited_rotation = {0.0f, 0.0f, 0.0f},
        .edited_scale = {0.0f, 0.0f, 0.0f},
    };

    ImGui::SetNextWindowPos(hierarchy_pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(hierarchy_size, ImGuiCond_FirstUseEver);
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

    ImGui::SetNextWindowPos(inspector_pos, ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(inspector_size, ImGuiCond_FirstUseEver);
    ImGui::Begin("Inspector");
    if (frame == nullptr || !frame->has_selected_entity)
    {
        ImGui::Text("No entity selected");
    }
    else 
    {
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
            float position[3];
            copy_float3(position, frame->selected_position);

            float rotation[3];
            copy_float3(rotation, frame->selected_rotation);

            float scale[3];
            copy_float3(scale, frame->selected_scale);

            bool changed = false;

            changed |= ImGui::DragFloat3("Position", position, 0.05f);
            changed |= ImGui::DragFloat3("Rotation", rotation, 0.5f);
            changed |= ImGui::DragFloat3("Scale", scale, 0.05f);

            if (changed)
            {
                result.transform_changed = true;

                copy_float3(result.edited_position, position);

                copy_float3(result.edited_rotation, rotation);

                copy_float3(result.edited_scale, scale);
            }
        }
        else 
        {
            ImGui::Text("No transform component");
        }

        if (frame->selected_light_type != EDITOR_SELECTED_LIGHT_NONE)
        { 
            ImGui::Separator();
            ImGui::Text("Light");

            float ambient[3];
            copy_float3(ambient, frame->selected_light_ambient);
            copy_float3(result.edited_light_ambient, ambient);

            float diffuse[3];
            copy_float3(diffuse, frame->selected_light_diffuse);
            copy_float3(result.edited_light_diffuse, diffuse);

            float specular[3];
            copy_float3(specular, frame->selected_light_specular);
            copy_float3(result.edited_light_specular, specular);

            float light_direction[3];
            copy_float3(light_direction, frame->selected_light_direction);
            copy_float3(result.edited_light_direction, light_direction);

            float light_position[3]; 
            copy_float3(light_position, frame->selected_light_position);
            copy_float3(result.edited_light_position, light_position);

            if (ImGui::ColorEdit3("Ambient", ambient))
            {
                result.light_changed = true;
                copy_float3(result.edited_light_ambient, ambient);
            }
            if (ImGui::ColorEdit3("Diffuse", diffuse))
            {
                result.light_changed = true;
                copy_float3(result.edited_light_diffuse, diffuse);
            }
            if (ImGui::ColorEdit3("Specular", specular))
            {
                result.light_changed = true;
                copy_float3(result.edited_light_specular, specular);
            }
            if (frame->selected_light_type == EDITOR_SELECTED_LIGHT_DIRECTIONAL && ImGui::DragFloat3("Light Direction", light_direction))
            {
                result.light_changed = true;
                copy_float3(result.edited_light_direction, light_direction);
            }
            if ((frame->selected_light_type == EDITOR_SELECTED_LIGHT_POINT || frame->selected_light_type == EDITOR_SELECTED_LIGHT_SPOT ) && ImGui::DragFloat3("Light Position", light_position))
            {
                result.light_changed = true;
                copy_float3(result.edited_light_position, light_position);
            }
            
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(stats_pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(stats_size, ImGuiCond_FirstUseEver);
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

    ImGui::SetNextWindowPos(camera_settings_pos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowSize(camera_settings_size, ImGuiCond_FirstUseEver);
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

    ImGui::SetNextWindowPos(scene_actions_pos, ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(scene_actions_size, ImGuiCond_FirstUseEver);
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
        if (frame->selected_entity_is_renderable)
        {
            if (ImGui::Button("Duplicate Selected"))
            {
                result.duplicate_selected_entity = true;
            }
        }
        if (ImGui::Button("Delete Selected"))
        {
            result.delete_selected_entity = true;
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