# Handoff — DS9C Complete; DS8 Next

Date: 2026-09-25
Completed deliverable: DS9C / RYA-44 — Engine/Editor Boundary And Separate Builds V0

## Repository Rules

The user manually edits all source code. The assistant may inspect source, run diagnostics, and edit documentation when asked. Do not edit source or perform Git state changes. Before work, inspect the related Linear issue. `notes/linear-updates.md` is historical only; use Linear directly for current status.

## DS9C Delivered

- `bour_engine` is a shared runtime library. `bour_game` and `bour_editor` are separate executable clients.
- The public runtime boundary is an opaque `EngineRuntime` plus C-owned inspection snapshots, mutation commands, input, and render-target models.
- The editor owns native window/event/UI state and talks to the runtime through that boundary; it no longer owns raw `Scene`, `Renderer`, or `Camera` state.
- Manual validation has passed for editor save/load, primitive creation, and asset-backed renderable creation through the runtime command path.
- CMake configuration rejects editor/ImGui files or includes from the `bour_engine` source and runtime-header boundary set.
- Standalone validation passed using `./src/build/bour_game test_scene.json`: saved-scene load/render, WASD/mouse camera input, no editor/ImGui initialization, and clean exit.
- `bour_game` contains no `imgui`, `editor_ui`, or `editor_app` symbols.
- RYA-44 is Done in Linear with build, standalone-validation, and symbol-scan evidence.

## Immediate Architectural Rules

- `bour_engine` never depends on an executable, `editor/`, or Dear ImGui.
- Runners own windows, event loops, swap/presentation, and native input collection. The runtime renders to caller-supplied targets.
- Editor preview remains the later DS9 responsibility: Play uses a disposable preview session and Stop discards temporary changes. DS9C establishes the boundary, not the full play lifecycle.
- Keep runtime modules small by responsibility: lifecycle/render, inspection, commands, and private state. Scene/ECS retain their own domain logic.

## Next Work After DS9C

- DS8 / RYA-17 is unblocked and is the next implementation target: editor-owned viewport picking and transform tools against the runtime boundary.
- RYA-93 refactors runtime commands to a tagged union before DS6 adds component/editing operations; it does not block DS8.
- The first-game target is a compact underwater discovery dive. The current delivery map and scope guards are in `notes/first-game-mvp-schedule.md`.
- Do not activate speculative post-MVP tooling. Complete only the bounded input, scene lifecycle, collision/query, game behavior, runtime UI/audio, animation, asset-reference, underwater-rendering, packaging, and game-specific items listed in the delivery audit.
