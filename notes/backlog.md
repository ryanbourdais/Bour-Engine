# Backlog

## Next Major Milestone: Editor Foundation

Build toward a usable editor by first separating engine state, scene data, and rendering responsibilities. The editor should eventually be able to inspect, select, create, move, and save objects without hardcoding a debug scene inside the renderer.

Odin, C++, and Zig evaluations are allowed during this milestone only when they directly support editor foundation work. The goal is to learn from small, contained subsystem trials without turning the editor milestone into a rewrite milestone.

## Milestone Exit Criteria

- Engine owns the application loop, timing, input, active camera, and active scene.
- Renderer consumes renderable scene data instead of owning hardcoded models/lights.
- A minimal ECS exists for entities, transforms, cameras, mesh renderers, and lights.
- A minimal UI/editor shell can run beside the viewport.
- A scene can be created, loaded, edited at a basic level, and saved.
- The LearnOpenGL Phong-era renderer work is complete through shadow maps.
- PBR is explicitly deferred to a later milestone.

## Epic 1: Engine And Renderer Boundary

Goal: finish the architecture work already started by moving camera/input ownership out of the renderer.

- [X] Remove the renderer global singleton or hide it behind explicit renderer state ownership.
- [X] Replace `renderer_render_frame(GLFWwindow *, Camera *)` with a render context or frame packet.
- [X] Move framebuffer-size queries out of per-frame renderer code where practical.
- [X] Give timing its own small abstraction.
- [X] Keep GLFW details isolated to window/platform-facing code.
- [X] Make renderer initialization accept configuration instead of hardcoded debug state.
- [X] Define the renderer’s input as scene/camera/light/renderable data, not gameplay state.

## Epic 2: Scene Management

Goal: create the runtime object model the editor will manipulate.

- [X] Introduce `Scene` as the owner of runtime entities and scene-level resources.
- [X] Add scene lifecycle: create, update, render-submit, destroy.
- [X] Move hardcoded model, skybox, cube positions, and debug lights into a scene setup path.
- [X] Add a default test scene that reproduces the current rendered output.
- [X] Add scene serialization format decision: [Decision: JSON, needs to be human readable for now] start simple, likely JSON or a custom text format. Note: this is the first point where a Zig trial may be useful; keep runtime `Scene` in C and evaluate Zig only for file parsing, validation, and round-trip tooling behind a plain C-facing boundary.
- [ ] Add scene save/load for transforms, model paths, light values, and active camera. Note: serialization/save-load can be a contained sub-subsystem or tool; it should feed C-owned runtime scene data rather than rewrite the runtime scene model.
- [ ] Add basic scene switching or scene reload during development.
- [ ] Optional evaluation: prototype scene serialization in Zig behind a plain C-facing boundary.

## Epic 3: ECS Foundation

Goal: introduce enough ECS to support editor workflows without overbuilding.

- [X] Define entity IDs and an entity registry.
- [X] Add core components: `Transform`, `Name`, `Camera`, `MeshRenderer`, `DirectionalLight`, `PointLight`, `SpotLight`.
- [X] Add component storage with create/get/remove/iterate operations.
- [ ] Add transform helpers for position, rotation, scale, and model matrix generation.
- [ ] Convert the current debug model/light setup into ECS entities.
- [ ] Add a render extraction step from ECS components into renderer-friendly draw data.
- [ ] Keep systems simple: transform update, camera update, render submission.
- [ ] Optional evaluation: prototype ECS component storage in Odin or Zig before committing to the C implementation.

## Epic 4: UI And Editor Shell

Goal: get the first usable editor surface on screen.

- [ ] Choose immediate-mode UI path, likely Dear ImGui unless there is a strong reason not to.
- [ ] If Dear ImGui is selected, evaluate whether the UI boundary should remain C-friendly or use a small C++ adapter.
- [ ] Add UI initialization/shutdown around the existing GLFW/OpenGL loop.
- [ ] Add editor mode toggle or editor executable entry path.
- [ ] Add dockable/simple panels: viewport, hierarchy, inspector, assets/log.
- [ ] Display FPS/frame timing in UI instead of only the window title.
- [ ] Add object selection from the hierarchy.
- [ ] Add inspector editing for transform and light values.
- [ ] Add basic viewport camera controls distinct from game camera behavior.

## Epic 5: Editor Interaction

Goal: make the editor useful, not just visible.

- [ ] Add entity create/delete/rename.
- [ ] Add transform editing through inspector fields.
- [ ] Add viewport object picking or temporary hierarchy-first selection.
- [ ] Add translate/rotate/scale gizmo support.
- [ ] Add asset/model path selection for mesh-renderer entities.
- [ ] Optional evaluation: build a small Zig or C++ asset/scene validation utility if it improves editor workflow.
- [ ] Add scene dirty state and save confirmation behavior.
- [ ] Add editor-safe play/update separation so editing does not require game simulation.

## Epic 6: Complete LearnOpenGL Phong Section

Goal: finish the pre-PBR rendering foundation before starting the PBR milestone.

- [ ] Resume at shadow maps.
- [ ] Implement directional shadow mapping.
- [ ] Add shadow map framebuffer/resource lifecycle.
- [ ] Add depth shader pass.
- [ ] Integrate shadows into the current Phong/Blinn-Phong shader path.
- [ ] Add debug view or toggle for the shadow map.
- [ ] Validate shadows with the current loaded model scene.
- [ ] Document what is complete and explicitly mark PBR as deferred.

## Later, Not This Milestone

- PBR materials and image-based lighting.
- Physics.
- Audio.
- Scripting.
- Networking.
- Distribution/build packaging.
- Advanced asset database/import pipeline.

## Following Milestone: Language Evaluation Spikes

Use small, contained subsystem trials to evaluate Odin, C++, and Zig as possible future implementation languages. Keep the C engine as the reference implementation until a trial clearly proves that another language improves clarity, iteration speed, maintainability, or subsystem design.

More detail lives in `notes/language-trials.md`.

## Language Evaluation Exit Criteria

- Each trial has a narrow subsystem boundary.
- Each trial exposes or plans a plain C-facing API.
- Each trial records build-system friction, debugging experience, memory ownership clarity, and API readability.
- No renderer-core or engine-loop rewrite happens before their current boundaries stabilize.
- The result is a recommendation, not an automatic rewrite.

## Evaluation Epic 1: Zig Scene Serialization

Goal: test Zig on a small but editor-relevant subsystem.

Note: this trial should start only after the C runtime scene has real data worth round-tripping. Zig is being evaluated for scene file parsing, validation, save/load, and tooling ergonomics, not for replacing the runtime `Scene` type during the editor foundation work.

- [ ] Define the minimal scene data that needs to round-trip.
- [ ] Serialize entity names, transforms, model paths, light values, and active camera.
- [ ] Load scene data into C-owned runtime structures or an intermediate C-compatible representation.
- [ ] Add save/load round-trip checks.
- [ ] Decide whether Zig should own scene serialization, remain a tooling language, or be dropped for this area.

## Evaluation Epic 2: Odin Or Zig ECS Prototype

Goal: compare data-oriented ECS storage ergonomics without destabilizing the main editor milestone.

- [ ] Prototype entity IDs and component storage outside the main runtime.
- [ ] Compare simple fixed-capacity arrays, packed arrays, and sparse-set style storage.
- [ ] Keep component shapes compatible with C structs.
- [ ] Measure whether the prototype makes iteration and ownership clearer than the C design.
- [ ] Feed lessons back into the main ECS implementation.

## Evaluation Epic 3: C++ Editor Boundary

Goal: evaluate C++ only where it has obvious leverage, especially around editor UI libraries.

- [ ] If Dear ImGui is selected, test a minimal C++ adapter around UI initialization, frame rendering, and shutdown.
- [ ] Keep the engine-facing surface plain C.
- [ ] Confirm CMake integration remains understandable.
- [ ] Decide whether C++ should own editor UI glue, editor tools, neither, or a later renderer-adjacent layer.

## Evaluation Epic 4: Asset Import And Validation Tooling

Goal: trial Zig or C++ on offline/editor-adjacent tools.

- [ ] Build a small model or asset manifest inspector.
- [ ] Validate model paths, texture references, and material metadata.
- [ ] Emit a simple intermediate manifest the editor can read later.
- [ ] Compare Zig and C++ fit for strings, paths, file IO, error reporting, and build integration.

## Evaluation Epic 5: Rewrite Decision Notes

Goal: make the rewrite question evidence-based.

- [ ] For each language, write what felt better than C.
- [ ] For each language, write what made the project harder.
- [ ] Identify which subsystems are good rewrite candidates.
- [ ] Identify which subsystems should stay C for now.
- [ ] Decide whether the next major milestone remains C-first, mixed-language, or targeted-rewrite.
