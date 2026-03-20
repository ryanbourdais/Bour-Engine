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
)

target_include_directories(bour_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_link_libraries(bour_engine PRIVATE
    glfw
    OpenGL::GL
)
