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

    ImGuiIO &io = ImGui::GetIO();
    ImVec2 display_size = io.DisplaySize;

    const float margin = 10.0f;

    const ImVec2 hierarchy_size = ImVec2(260, 300);
    const ImVec2 stats_size = ImVec2(260, 180);
    const ImVec2 inspector_size = ImVec2(320, 300);
    const ImVec2 scene_actions_size = ImVec2(320, 140);
    const ImVec2 camera_settings_size = ImVec2(260, 100);

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

    ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(hierarchy_size, ImGuiCond_Always);
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


    ImGui::SetNextWindowPos(ImVec2(display_size.x - margin, margin), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(inspector_size, ImGuiCond_Always);
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

            if (frame->selected_light_type != EDITOR_SELECTED_LIGHT_NONE)
            {
                ImGui::Separator();
                ImGui::Text("Light");

                float ambient[3] = {
                    frame->selected_light_ambient[0],
                    frame->selected_light_ambient[1],
                    frame->selected_light_ambient[2]
                };
                float diffuse[3] = {
                    frame->selected_light_diffuse[0],
                    frame->selected_light_diffuse[1],
                    frame->selected_light_diffuse[2]
                };
                float light_direction[3] = {
                    frame->selected_light_direction[0],
                    frame->selected_light_direction[1],
                    frame->selected_light_direction[2]
                };
                float light_position[3] = {
                    frame->selected_light_position[0],
                    frame->selected_light_position[1],
                    frame->selected_light_position[2]
                };
                float specular[3] = {
                    frame->selected_light_specular[0],
                    frame->selected_light_specular[1],
                    frame->selected_light_specular[2]
                };

                result.edited_light_ambient[0] = ambient[0];
                result.edited_light_ambient[1] = ambient[1];
                result.edited_light_ambient[2] = ambient[2];

                result.edited_light_diffuse[0] = diffuse[0];
                result.edited_light_diffuse[1] = diffuse[1];
                result.edited_light_diffuse[2] = diffuse[2];

                result.edited_light_specular[0] = specular[0];
                result.edited_light_specular[1] = specular[1];
                result.edited_light_specular[2] = specular[2];

                result.edited_light_direction[0] = light_direction[0];
                result.edited_light_direction[1] = light_direction[1];
                result.edited_light_direction[2] = light_direction[2];

                result.edited_light_position[0] = light_position[0];
                result.edited_light_position[1] = light_position[1];
                result.edited_light_position[2] = light_position[2];

                if (ImGui::ColorEdit3("Ambient", ambient))
                {
                    result.light_changed = true;
                    result.edited_light_ambient[0] = ambient[0];
                    result.edited_light_ambient[1] = ambient[1];
                    result.edited_light_ambient[2] = ambient[2];
                }
                if (ImGui::ColorEdit3("Diffuse", diffuse))
                {
                    result.light_changed = true;
                    result.edited_light_diffuse[0] = diffuse[0];
                    result.edited_light_diffuse[1] = diffuse[1];
                    result.edited_light_diffuse[2] = diffuse[2];
                }
                if (ImGui::ColorEdit3("Specular", specular))
                {
                    result.light_changed = true;
                    result.edited_light_specular[0] = specular[0];
                    result.edited_light_specular[1] = specular[1];
                    result.edited_light_specular[2] = specular[2];
                }
                if (frame->selected_light_type == EDITOR_SELECTED_LIGHT_DIRECTIONAL && ImGui::DragFloat3("Light Direction", light_direction))
                {
                    result.light_changed = true;
                    result.edited_light_direction[0] = light_direction[0];
                    result.edited_light_direction[1] = light_direction[1];
                    result.edited_light_direction[2] = light_direction[2];
                }
                if ((frame->selected_light_type == EDITOR_SELECTED_LIGHT_POINT || frame->selected_light_type == EDITOR_SELECTED_LIGHT_SPOT ) && ImGui::DragFloat3("Light Position", light_position))
                {
                    result.light_changed = true;
                    result.edited_light_position[0] = light_position[0];
                    result.edited_light_position[1] = light_position[1];
                    result.edited_light_position[2] = light_position[2];
                }
                
            }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(margin, margin + hierarchy_size .y + margin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(stats_size, ImGuiCond_Always);
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

    ImGui::SetNextWindowPos(ImVec2(display_size.x * 0.5f, display_size.y - margin), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowSize(camera_settings_size, ImGuiCond_Always);
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

    ImGui::SetNextWindowPos(ImVec2(display_size.x - margin, margin + inspector_size.y + margin), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(scene_actions_size, ImGuiCond_Always);
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