# Renderer Basics

This document is a reference for the current renderer architecture and the core vocabulary behind it.

It is meant to help you read your own codebase and understand the terms you will keep seeing as the renderer grows.

## Current Architecture

The renderer is currently split into a few focused modules.

### `engine`

Files:

- `src/engine/engine.c`
- `src/engine/engine.h`

Responsibility:

- application startup, frame loop, and shutdown coordination
- initializes GLFW
- sets GLFW hints
- calls into the window layer
- terminates GLFW on exit

Engine owns the active runtime camera, scene, and editor coordination. Renderer consumes frame data.

### `window`

Files:

- `src/renderer/window.c`
- `src/renderer/window.h`

Responsibility:

- creates the GLFW window
- creates the OpenGL context
- installs callbacks
- initializes GLAD
- exposes window lifecycle, events, dimensions, and presentation to Engine
- destroys the window

This module owns the lifetime of the actual window and GL context.

### `renderer`

Files:

- `src/renderer/renderer.c`
- `src/renderer/renderer.h`

Responsibility:

- owns renderer state
- initializes render resources
- renders one frame supplied by Engine
- shuts renderer resources down

This is the module that coordinates actual drawing.

### `shaders`

Files:

- `src/renderer/shaders.c`
- `src/renderer/shaders.h`

Responsibility:

- reads shader files
- compiles shaders
- links the shader program
- reports shader compile and link errors

This module owns shader-related OpenGL setup.

### `mesh`

Files:

- `src/renderer/data_types/mesh.c`
- `src/renderer/data_types/mesh.h`

Responsibility:

- uploads supplied vertex/index data
- creates the VBO
- creates the VAO
- creates the EBO and releases owned geometry handles through `mesh_free()`

`vertex.h` defines position, color, UV, and normal fields. Mesh initialization establishes an empty state; it must not overwrite a live mesh without cleanup. Materials/textures have separate ownership.

### Built-in primitives (2026-09-07)

`src/geometry/primitive_types.h` defines neutral primitive identifiers. `primitive_mesh.c` exposes borrowed static cube/plane/quad CPU arrays. `primitive_mesh_resources.c` uploads and owns a collection of GPU meshes. Renderer owns this collection's lifecycle; Scene translates ECS mesh sources into renderer draw data.

The plane and quad share the same four-vertex, two-triangle topology, but their default orientation is intentionally different: a plane lies in XZ with a `+Y` front normal for ground-like surfaces; a quad lies in XY with a `+Z` front normal for wall, panel, or billboard-like surfaces. Both are single-sided under the renderer's CCW front-face and back-face-culling convention. The plane indices therefore wind counter-clockwise when viewed from `+Y`.

Primitive resources have a renderer-owned opaque white fallback material and draw through normal scene submission. Primitive-only renderer startup is supported. Sphere/cylinder and programmable geometry are not implemented. Mesh upload does not yet detect OpenGL allocation/upload errors.

### `file_reader`

Files:

- `src/utils/helpers/file_reader.c`
- `src/utils/helpers/file_reader.h`

Responsibility:

- reads shader source files into memory
- returns a simple owned buffer

This module is not renderer-specific in theory, but it is currently used by the shader system.

## Current Flow

The program currently works roughly like this:

1. `main()` calls `engine_run()`.
2. Engine initializes GLFW, creates the window/context, and initializes camera, Scene, Renderer, and Editor.
3. Engine updates input/camera/scene and extracts render data, then gathers and applies editor commands.
4. `renderer_render_frame()` draws into the MSAA scene target, resolves to a texture, and presents it through a screen quad. Engine renders editor UI and presents the window.
5. Engine shuts down renderer resources before destroying the OpenGL context.

That is the current high-level architecture.

## Core Vocabulary

These are the main graphics terms you should be comfortable with in this codebase.

### Render

To render means to produce the final image for the current frame.

In practice, this usually means:

- clear the framebuffer
- bind the right GPU state
- issue draw calls
- present the result

`render` usually refers to frame production or renderer behavior at a high level.

### Draw

To draw means to issue a GPU draw command.

Examples:

- `glDrawArrays(...)`
- `glDrawElements(...)`

`draw` is usually narrower than `render`.

A frame may contain many draw calls, but rendering a frame is broader than any single one of them.

### Render Loop

The render loop is the repeating loop that keeps the application drawing frames until the window closes.

Typical loop steps:

- poll input/events
- clear the screen
- bind resources
- draw geometry
- swap buffers

In this project, Engine owns the repeating application loop and calls Renderer once per frame.

### Frame

A frame is one complete image produced by the renderer.

If your application is running at 60 FPS, it is producing about 60 frames per second.

### FPS

FPS means frames per second.

It is a simple measurement of how often the renderer is producing frames.

### Shader

A shader is a GPU program.

Your current code uses:

- a vertex shader
- a fragment shader

These are compiled and linked into a shader program before drawing.

### Vertex Shader

The vertex shader runs once per vertex.

Its main job is to transform vertex data into clip-space positions by writing to:

- `gl_Position`

The current light vertex shader applies model/view/projection transforms and transforms normals for lighting.

### Fragment Shader

The fragment shader runs for fragments/pixels generated by rasterization.

Its main job is usually to output a final color.

The current light fragment shader combines material texture/color with scene lighting.

### Shader Program

A shader program is the linked combination of compiled shaders.

You typically:

1. create individual shaders
2. compile them
3. attach them to a program
4. link the program
5. use the program when drawing

### Compile vs Link

Compile:

- checks whether an individual shader is valid

Link:

- checks whether compiled shaders work together as a complete program

A shader can compile successfully but still fail to link with another shader.

### Vertex

A vertex is one point of input geometry.

The current `Vertex` contains:

- position
- color
- UV coordinates
- normal

### VAO

VAO means Vertex Array Object.

A VAO stores how vertex input should be interpreted.

It records things like:

- which vertex attributes are enabled
- where vertex data comes from
- how the data is laid out

Think of it as vertex-input configuration state.

### VBO

VBO means Vertex Buffer Object.

A VBO stores raw vertex data on the GPU.

The current mesh uploader stores interleaved `Vertex` records in a VBO and indices in an EBO.

### Attribute

A vertex attribute is one piece of per-vertex data given to the vertex shader.

For example:

- position
- normal
- UV coordinates
- color

In your current example, attribute location `0` is the vertex position.

### OpenGL Context

The OpenGL context is the stateful environment where OpenGL commands operate.

You need a current context before calling most GL functions.

In your project, the window layer makes the context current before starting GLAD and rendering.

### GLAD

GLAD loads OpenGL function pointers.

Without it, modern OpenGL functions like `glCreateShader` or `glBindVertexArray` would not be available through normal function calls.

### GLFW

GLFW handles:

- window creation
- input/events
- context creation
- buffer swapping

It is the platform/windowing layer in your project.

### Viewport

The viewport tells OpenGL how to map normalized device coordinates to the framebuffer.

When the window resizes, you update the viewport so OpenGL knows the new drawable area.

Correct resizing must keep projection aspect ratio, offscreen attachment sizes, resolve dimensions, and viewport dimensions consistent. The current callback only updates the viewport; complete resizing is tracked in DS3B.

### Framebuffer

The framebuffer is the image target OpenGL draws into.

When you clear the screen, you are clearing the framebuffer.

When you swap buffers, the rendered framebuffer becomes visible.

### Aspect Ratio

Aspect ratio is width divided by height.

If you do not correct for aspect ratio, shapes can look stretched when the window changes shape.

### State

In graphics code, "state" usually means currently active OpenGL configuration or your own renderer-owned data.

Examples:

- currently bound shader program
- currently bound VAO
- current clear color
- renderer-owned GL handles

### State Struct

A state struct is a C struct that groups related data with the same ownership and lifetime.

In your current code, `RendererState` groups renderer-owned OpenGL objects like:

- shader program
- VAO
- VBO

This makes ownership and cleanup clearer than passing many loose handles around.

## Naming Guidance

The current codebase is aiming to use these meanings consistently:

- `run_*`: owns a loop or subsystem lifetime
- `render_*`: owns renderer-level behavior or frame production
- `draw_*`: issues draw calls
- `init_*`: creates or initializes long-lived state
- `shutdown_*` or `destroy_*`: releases long-lived state

This is not a rigid rule, but it is a good default.

## What To Keep In Mind Going Forward

Already implemented concepts include:

- projection matrices
- camera state
- uniforms
- textures
- index buffers
- multiple meshes
- multiple shader programs
- scene-level state

Current work connects engine-owned primitives to the draw path. Editor creation/persistence and primitive-only startup remain open; shadow maps are a later deliverable and PBR is deferred. See `docs/arch.md` for current ownership limitations and `notes/backlog.md` for progress.
