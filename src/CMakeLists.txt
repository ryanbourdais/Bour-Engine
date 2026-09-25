cmake_minimum_required(VERSION 3.20)

project(bour_engine C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_subdirectory(external/cglm EXCLUDE_FROM_ALL)

set(BOUR_ENGINE_SOURCES
      engine/engine_runtime.c
      engine/engine_runtime_inspection.c
      engine/engine_runtime_commands.c
      external/glad/src/glad.c
      external/stb/stb_image.c
      external/cgltf/cgltf.c
      utils/helpers/file_reader.c
      utils/math_utils.c
      renderer/shaders.c
      renderer/data_types/mesh.c
      renderer/renderer.c
      renderer/data_types/texture.c
      renderer/data_types/lightObject.c
      renderer/data_types/material.c
      renderer/data_types/model.c
      renderer/data_types/skybox.c
      renderer/data_types/renderTarget.c
      renderer/data_types/primitive_mesh.c
      renderer/data_types/primitive_mesh_resources.c
      renderer/data_types/programmable_mesh_resources.c
      renderer/camera.c
      scene/scene.c
      scene/entity_factory.c
      ecs/entity.c
      ecs/components.c
      ecs/component_storage.c
      geometry/programmable_mesh.c
      scene/scene_serialization.c
      external/jsmn/jsmn.c
  )

  set(BOUR_ENGINE_BOUNDARY_HEADERS
      engine/engine_runtime.h
      engine/engine_runtime_internal.h
  )

  set(BOUR_ENGINE_BOUNDARY_FILES
      ${BOUR_ENGINE_SOURCES}
      ${BOUR_ENGINE_BOUNDARY_HEADERS}
  )

  foreach(boundary_file IN LISTS BOUR_ENGINE_BOUNDARY_FILES)
        if(boundary_file MATCHES "(^|/)editor/" OR
            boundary_file MATCHES "(^|/)external/imgui/")
                message(FATAL_ERROR
                    "bour_engine must not compile editor or Dear ImGui file: "
                    "${boundary_file}"
                )
        endif()

        file(READ
            "${CMAKE_CURRENT_SOURCE_DIR}/${boundary_file}"
            boundary_file_contents
        )

        string(REGEX MATCH
            "#[ \t]*include[ \t]*[<\"][^>\"]*(editor|imgui)"
            forbidden_runtime_include
            "${boundary_file_contents}"
        )

        if(forbidden_runtime_include)
            message(FATAL_ERROR
                "bour_engine boundary file must not include editor or Dear ImGUi: "
                "${boundary_file}"
            )
        endif()
endforeach()

add_library(bour_engine STATIC ${BOUR_ENGINE_SOURCES})

target_include_directories(bour_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/cgltf
    ${CMAKE_CURRENT_SOURCE_DIR}/external/jsmn
)

target_link_libraries(bour_engine PUBLIC
    OpenGL::GL
    cglm_headers
    m
)

add_executable(bour_game
    game/game_main.c
    renderer/window.c
)

target_include_directories(bour_game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_link_libraries(bour_game PRIVATE
    bour_engine
    glfw
)

add_executable(bour_editor
    main.c
    editor/editor_app.c
    engine/timing.c
    utils/profiler.c
    renderer/window.c
    controller/input.c
    editor/editor_ui.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(bour_editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/cgltf
    ${CMAKE_CURRENT_SOURCE_DIR}/external/jsmn
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

target_link_libraries(bour_editor PRIVATE
    bour_engine
    glfw
)
