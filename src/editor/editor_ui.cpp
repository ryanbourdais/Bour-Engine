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

    const float scene_toolbar_height = 36.0f;

    const float side_pane_height =
        display_size.y - (margin * 2.0f);

    static float hierarchy_width = 260.0f;
    static float right_pane_width = 320.0f;
    static float inspector_height = 0.0f;

    if (inspector_height <= 0.0f)
    {
        inspector_height = side_pane_height - 220.0f - margin;
    }

    const float stats_height =
        side_pane_height - inspector_height - margin;

    const ImVec2 hierarchy_size =
        ImVec2(hierarchy_width, side_pane_height);

    const ImVec2 inspector_size =
        ImVec2(right_pane_width, inspector_height);
    
    const ImVec2 stats_size =
        ImVec2(right_pane_width, stats_height);

    const float minimum_scene_width = 320.0f;
    const float minimum_pane_width = 180.0f;

    const float maximum_hierarchy_width_candidate =
        display_size.x -
        right_pane_width -
        minimum_scene_width -
        (margin * 3.0f);

    const float maximum_hierarchy_width =
        maximum_hierarchy_width_candidate > minimum_pane_width
            ? maximum_hierarchy_width_candidate
            : minimum_pane_width;

    const float maximum_right_pane_width_candidate =
        display_size.x -
        hierarchy_width -
        minimum_scene_width -
        (margin * 3.0f);

    const float maximum_right_pane_width =
        maximum_right_pane_width_candidate > minimum_pane_width
            ? maximum_right_pane_width_candidate    
            : minimum_pane_width;

    const float minimum_inspector_height = 180.0f;
    const float minimum_stats_height = 160.0f;

    const float maximum_inspector_height_candidate =
        side_pane_height - minimum_stats_height - margin;

    const float maximum_inspector_height =
        maximum_inspector_height_candidate > minimum_inspector_height
            ? maximum_inspector_height_candidate
            : minimum_inspector_height;

    const ImVec2 hierarchy_pos = ImVec2(margin, margin);
    const ImVec2 inspector_pos = ImVec2(display_size.x - margin, margin);
    const ImVec2 stats_pos = ImVec2(
        display_size.x - margin,
        margin + inspector_size.y + margin
    );

    const ImGuiWindowFlags fixed_window_flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize;

    EditorFrameResult result = {
        .selected_entity_id = frame != nullptr ? frame->selected_entity_id : 0,
        .selection_changed = false,
        .toggle_editor_cursor = false,
        .save_scene = false,
        .load_scene = false,
        .rename_selected_entity = false,
        .edited_name = {0},
        .create_empty_entity = false,
        .create_renderable_entity = false,
        .create_primitive_entity = false,
        .create_programmable_plane = false,
        .primitive_type_to_create = BUILTIN_PRIMITIVE_COUNT,
        .duplicate_selected_entity = false,
        .delete_selected_entity = false,
        .transform_changed = false,
        .edited_position = {0.0f, 0.0f, 0.0f},
        .edited_rotation = {0.0f, 0.0f, 0.0f},
        .edited_scale = {0.0f, 0.0f, 0.0f},
    };

    const ImVec2 scene_toolbar_pos =
        ImVec2(margin + hierarchy_size.x + margin, margin);

    const ImVec2 scene_view_pos =
        ImVec2(
            margin + hierarchy_size.x + margin,
            margin + scene_toolbar_height + margin
        );

    const float available_scene_width =
        display_size.x - hierarchy_size.x - inspector_size.x - (margin * 3.0f);

    const float available_scene_height =
        display_size.y - scene_view_pos.y - margin;

    const ImVec2 scene_view_size = ImVec2(
        available_scene_width > 1.0f ? available_scene_width : 1.0f,
        available_scene_height > 1.0f ? available_scene_height : 1.0f
    );

    const ImVec2 scene_toolbar_size = ImVec2(
        available_scene_width > 1.0f ? available_scene_width : 1.0f,
        scene_toolbar_height
    );

    ImGui::SetNextWindowPos(scene_toolbar_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(scene_toolbar_size, ImGuiCond_Always);
    ImGui::Begin(
        "Scene Toolbar",
        nullptr,
        fixed_window_flags |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar
    );

    if (frame != nullptr)
    {
        ImGui::Text(
            "Mouse: %s",
            frame->editor_cursor_enabled ? "Editor" : "Camera"
        );
        ImGui::SameLine();

        if (ImGui::Button(
                frame->editor_cursor_enabled
                    ? "Use Camera Mouse"
                    : "Use Editor Mouse"
            ))
        {
            result.toggle_editor_cursor = true;
        }
    }

    ImGui::SameLine();

    if (ImGui::BeginMenu("Scene"))
    {
         if (ImGui::BeginMenu("Create"))
          {
              if (ImGui::MenuItem("Empty Entity"))
              {
                  result.create_empty_entity = true;
              }
              if (ImGui::MenuItem("Renderable Entity"))
              {
                  result.create_renderable_entity = true;
              }
              if (ImGui::MenuItem("Cube"))
              {
                  result.create_primitive_entity = true;
                  result.primitive_type_to_create = BUILTIN_PRIMITIVE_CUBE;
              }
              if (ImGui::MenuItem("Plane"))
              {
                  result.create_primitive_entity = true;
                  result.primitive_type_to_create = BUILTIN_PRIMITIVE_PLANE;
              }
              if (ImGui::MenuItem("Quad"))
              {
                  result.create_primitive_entity = true;
                  result.primitive_type_to_create = BUILTIN_PRIMITIVE_QUAD;
              }
              if (ImGui::MenuItem("UV Sphere"))
              {
                  result.create_primitive_entity = true;
                  result.primitive_type_to_create = BUILTIN_PRIMITIVE_UV_SPHERE;
              }
              if (ImGui::MenuItem("Cylinder"))
              {
                  result.create_primitive_entity = true;
                  result.primitive_type_to_create = BUILTIN_PRIMITIVE_CYLINDER;
              }
              if (ImGui::MenuItem("Programmable Plane"))
              {
                  result.create_programmable_plane = true;
              }

              ImGui::EndMenu();
        }
        if (frame != nullptr && frame->has_selected_entity)
        {
            if (frame->selected_entity_is_renderable &&
                ImGui::MenuItem("Duplicate Selected"))
            {
                result.duplicate_selected_entity = true;
            }

            if (ImGui::MenuItem("Delete Selected"))
            {
                result.delete_selected_entity = true;
            }
        }

      ImGui::Separator();

      if (ImGui::MenuItem("Save Scene"))
      {
          result.save_scene = true;
      }
      if (ImGui::MenuItem("Load Scene"))
      {
          result.load_scene = true;
      }

      ImGui::EndMenu();
    }

    ImGui::End();

    ImGui::SetNextWindowPos(scene_view_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(scene_view_size, ImGuiCond_Always);
    ImGui::Begin(
        "Scene View",
        nullptr,
        ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize
    );

    const ImVec2 viewport_pos = ImGui::GetCursorScreenPos();
    const ImVec2 viewport_size = ImGui::GetContentRegionAvail();

    result.viewport.x = viewport_pos.x;
    result.viewport.y = viewport_pos.y;
    result.viewport.width = viewport_size.x;
    result.viewport.height = viewport_size.y;
    result.viewport.framebuffer_width =
        (int)(viewport_size.x * io.DisplayFramebufferScale.x);
    result.viewport.framebuffer_height =
        (int)(viewport_size.y * io.DisplayFramebufferScale.y);
    result.viewport.hovered = ImGui::IsWindowHovered();
    result.viewport.focused = ImGui::IsWindowFocused();
    result.viewport.resolved_scene_texture =
        frame != nullptr ? frame->resolved_scene_texture : 0;

    if (result.viewport.resolved_scene_texture != 0)
    {
        ImGui::Image(
            ImTextureRef(
                (ImTextureID)result.viewport.resolved_scene_texture
            ),
            viewport_size,
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f)
        );
    }
    else {
        ImGui::Dummy(viewport_size);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(hierarchy_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(hierarchy_size, ImGuiCond_Always);
    ImGui::Begin("Hierarchy", nullptr, fixed_window_flags);

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

    ImGui::SetNextWindowPos(inspector_pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(inspector_size, ImGuiCond_Always);
    ImGui::Begin("Inspector", nullptr, fixed_window_flags);
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

        if (frame->selected_entity_is_programmable_mesh)
        {
            ImGui::Separator();
            ImGui::Text("Programmable Mesh");
            ImGui::Text("Type: Plane");
            ImGui::Text(
                "Runtime Mesh ID: %u",
                frame->selected_programmable_mesh_id
            );
            ImGui::Text(
                "Dimensions: %.3f x %.3f",
                frame->selected_programmable_plane_width,
                frame->selected_programmable_plane_depth
            );
            ImGui::Text(
                "GPU Sync: %s",
                frame->selected_programmable_mesh_dirty
                    ? "Pending upload"
                    : "Current"
            );
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

    ImGui::SetNextWindowPos(stats_pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(stats_size, ImGuiCond_Always);
    ImGui::Begin("Stats", nullptr, fixed_window_flags);
    if (frame != nullptr)
    {
        ImGui::Text("FPS: %.2f", frame->fps);
        ImGui::Text("Delta: %.3f ms", frame->delta_time * 1000.0);
        ImGui::Text("Entities: %zu", frame->entity_count);
        ImGui::Text("Renderables: %zu", frame->renderable_count);

        ImGui::Separator();
        ImGui::Text("Loaded Meshes: %zu", frame->renderer_mesh_count);
        ImGui::Text("Loaded Vertices: %zu", frame->renderer_vertex_count);
        ImGui::Text("Loaded Triangles: %zu", frame->renderer_triangle_count);
        ImGui::Text("Loaded Textures: %zu", frame->renderer_texture_count);

        ImGui::Separator();
        ImGui::Text("Submitted Draws: %zu", frame->renderer_submitted_draw_count);
        ImGui::Text("Submitted Meshes: %zu", frame->renderer_submitted_mesh_count);    
        ImGui::Text("Submitted Vertices: %zu", frame->renderer_submitted_vertex_count);    
        ImGui::Text("Submitted Triangles: %zu", frame->renderer_submitted_triangle_count);    
        ImGui::Text("Missing Models: %zu", frame->renderer_missing_model_count);    
    }

    ImGui::Separator();
    ImGui::Text("Engine Update: %.3f ms", frame->profile_engine_update_ms);
    ImGui::Text("Scene Extract: %.3f ms", frame->profile_scene_extract_ms);
    ImGui::Text("Editor Begin: %.3f ms", frame->profile_editor_begin_ms);
    ImGui::Text("Renderer: %.3f ms", frame->profile_renderer_ms);
    ImGui::Text("Editor Render: %.3f ms", frame->profile_editor_render_ms);
    ImGui::Text("Present: %.3f ms", frame->profile_present_ms);

    ImGui::End();

    enum EditorActiveSplitter {
        EDITOR_ACTIVE_SPLITTER_NONE = 0,
        EDITOR_ACTIVE_SPLITTER_HIERARCHY,
        EDITOR_ACTIVE_SPLITTER_RIGHT_PANE,
        EDITOR_ACTIVE_SPLITTER_INSPECTOR_STATS,
    };

    static EditorActiveSplitter active_splitter =
        EDITOR_ACTIVE_SPLITTER_NONE;

    const float splitter_hit_width = 8.0f;

    const float hierarchy_splitter_x =
        margin + hierarchy_width;

    const float right_pane_splitter_x =
        display_size.x - margin - right_pane_width;

    const float inspector_stats_splitter_y =
        margin + inspector_height;

    const bool mouse_within_pane_height =
        io.MousePos.y >= margin &&
        io.MousePos.y <= display_size.y - margin;

    const bool hovering_hierarchy_splitter =
        mouse_within_pane_height &&
        io.MousePos.x >=
            hierarchy_splitter_x - (splitter_hit_width * 0.5f) &&
        io.MousePos.x <=
            hierarchy_splitter_x + (splitter_hit_width * 0.5f);

    const bool hovering_right_pane_splitter =
        mouse_within_pane_height &&
        io.MousePos.x >=
            right_pane_splitter_x - (splitter_hit_width * 0.5f) &&
        io.MousePos.x <=
            right_pane_splitter_x + (splitter_hit_width * 0.5f);

    const bool hovering_inspector_stats_splitter =
        io.MousePos.x >=
            right_pane_splitter_x + (splitter_hit_width * 0.5f) &&
        io. MousePos.x <= display_size.x - margin &&
        io.MousePos.y >=
            inspector_stats_splitter_y - (splitter_hit_width * 0.5f) &&
        io.MousePos.y <=
            inspector_stats_splitter_y + (splitter_hit_width * 0.5f);

    if (active_splitter == EDITOR_ACTIVE_SPLITTER_NONE &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (hovering_hierarchy_splitter)
        {
            active_splitter = EDITOR_ACTIVE_SPLITTER_HIERARCHY;
        }
        else if (hovering_right_pane_splitter)
        {
           active_splitter = EDITOR_ACTIVE_SPLITTER_RIGHT_PANE;
        }
        else if (hovering_inspector_stats_splitter)
        {
            active_splitter =
                EDITOR_ACTIVE_SPLITTER_INSPECTOR_STATS;
        }
    }

    if (active_splitter == EDITOR_ACTIVE_SPLITTER_HIERARCHY)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            hierarchy_width += io.MouseDelta.x;

            if (hierarchy_width < minimum_pane_width)
            {
                hierarchy_width = minimum_pane_width;
            }
            else if (hierarchy_width > maximum_hierarchy_width)
            {
                hierarchy_width = maximum_hierarchy_width;
            }
        }
        else
        {
            active_splitter = EDITOR_ACTIVE_SPLITTER_NONE;
        }
    }
    else if (active_splitter == EDITOR_ACTIVE_SPLITTER_RIGHT_PANE)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            right_pane_width -= io.MouseDelta.x;

            if (right_pane_width < minimum_pane_width)
            {
                right_pane_width = minimum_pane_width;
            }
            else if (right_pane_width > maximum_right_pane_width)
            {
                right_pane_width = maximum_right_pane_width;
            }
        }
        else 
        {
            active_splitter = EDITOR_ACTIVE_SPLITTER_NONE;
        }
    }
    else if (active_splitter == EDITOR_ACTIVE_SPLITTER_INSPECTOR_STATS)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            inspector_height += io.MouseDelta.y;

            if (inspector_height < minimum_inspector_height)
            {
                inspector_height = minimum_inspector_height;
            }
            else if (inspector_height > maximum_inspector_height)
            {
                inspector_height = maximum_inspector_height;
            }
        }
        else
        {
            active_splitter = EDITOR_ACTIVE_SPLITTER_NONE;
        }
    }
    else if (hovering_inspector_stats_splitter)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    }
    else if (hovering_hierarchy_splitter ||
             hovering_right_pane_splitter)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

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
