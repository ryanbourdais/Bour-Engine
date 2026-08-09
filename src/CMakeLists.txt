cmake_minimum_required(VERSION 3.20)

project(bour_engine C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(bour_engine
    main.c
    engine/engine.c
    external/glad/src/glad.c
    external/stb/stb_image.c
    external/cgltf/cgltf.c
    utils/helpers/file_reader.c
    utils/math_utils.c
    utils/profiler.c
    renderer/window.c
    renderer/shaders.c
    renderer/data_types/mesh.c
    renderer/renderer.c
    controller/input.c
    renderer/data_types/texture.c
    renderer/data_types/lightObject.c
    renderer/data_types/material.c
    renderer/data_types/model.c
    renderer/data_types/skybox.c
    renderer/data_types/renderTarget.c
    renderer/camera.c
    engine/timing.c
    scene/scene.c
    ecs/entity.c
    ecs/components.c
    ecs/component_storage.c
    editor/editor_ui.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_subdirectory(external/cglm EXCLUDE_FROM_ALL)


target_include_directories(bour_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/cgltf
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

target_link_libraries(bour_engine PRIVATE
    glfw
    OpenGL::GL
    cglm_headers
)
