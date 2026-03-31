cmake_minimum_required(VERSION 3.20)

project(bour_engine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(bour_engine
    main.c
    renderer/rendering_engine.c
    external/glad/src/glad.c
    external/stb/stb_image.c
    utils/helpers/file_reader.c
    renderer/window.c
    renderer/shaders.c
    renderer/data_types/mesh.c
    renderer/renderer.c
    controller/input.c
    renderer/data_types/vertex.c
    renderer/data_types/vectors.c
    renderer/data_types/texture.c
    renderer/data_types/renderObject.c
)

add_subdirectory(external/cglm EXCLUDE_FROM_ALL)


target_include_directories(bour_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_link_libraries(bour_engine PRIVATE
    glfw
    OpenGL::GL
    cglm_headers
)
