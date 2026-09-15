# Bour Engine
A from-scratch game engine written in C, using OpenGL via GLFW and GLAD.

This project is focused on learning low-level graphics programming, engine architecture, and building systems without relying on heavy frameworks or AI-generated code.

## Goals
- Build a modular, extensible C-first engine with narrow adapters where needed
- Understand graphics programming fundamentals (not just use them)
- Develop clean subsystem architecture (renderer, input, etc.)
- Avoid black-box abstractions
- Maintain full control over memory and data structures

## Build Philosophy
- **No AI-written code** — everything is written and understood manually
- **AI-assisted documentation and planning** — AI may help organize notes, backlog, and project management
- **Learn by building** — not by stitching together libraries
- **Keep it simple first** — complexity comes later
- **Working > Perfect** — every milestone must run

## Tech Stack
- Language: C11
- Build System: CMake 3.20+
- Graphics API: OpenGL (4.x)
- Windowing/Input: GLFW3 (3.4)
- OpenGL Loader: GLAD (0.1.36)

## Getting Started
Getting started docs can be found for both Windows and Macbook systems in [init.md](https://github.com/ryanbourdais/Bour-Engine/blob/main/docs/init.md)

## Current Architecture

Engine coordinates the application; Scene owns authoring state; ECS stores components; Renderer owns GPU resources; Editor returns requested edits through a C-facing API. Dear ImGui uses a small C++ adapter. See [architecture](docs/arch.md) for ownership and current limitations.

```
src/
├── main.c          # Entry point
├── engine/         # Application lifecycle, frame loop, editor coordination
├── scene/          # Runtime scene, render extraction, JSON persistence
├── ecs/            # Entity registry and component storage
├── geometry/       # Neutral built-in primitive identifiers
├── editor/         # Dear ImGui C++ adapter with a C-facing API
├── controller/     # Input handling
├── external/       # Vendored dependencies and cglm submodule
├── renderer/       # Rendering system (OpenGL)
│   ├── data_types/ # Mesh/model/material and primitive GPU resources
│   └── shaders/    # GLSL shaders
├── utils/          # Shared utilities
```

## License
This project is licensed under the Apache License 2.0. See [LICENSE](LICENSE).

Third-party dependencies retain their original licenses.
