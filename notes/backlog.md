# Backlog

## Current Milestone: Editor Foundation

Commit 100 completed the first hierarchy-first editor alpha. The milestone is still active: the editor now exists, but it cannot yet author reusable scene data or support a clean scene-editing workflow.

The backlog is organized as deliverable sets instead of broad epics. Each deliverable set should be small enough to become one branch or a short sequence of related branches. If a task does not contribute to the deliverable set's exit criteria, move it to a later set instead of expanding the branch.

Current language direction: keep engine/runtime architecture C-first. Odin is the chosen direction for future game-level scripting/customization above the engine core with Lua as a backup/secondary scripting language. Dear ImGui is the locked-in editor UI path for the current editor foundation milestone.

## Milestone Exit Criteria

Editor Foundation is complete when:

- Engine owns the application loop, timing, input, active camera, and active scene.
- Renderer consumes renderable scene data instead of owning hardcoded models/lights.
- A minimal ECS exists for entities, transforms, cameras, mesh renderers, and lights.
- A minimal UI/editor shell can run beside the viewport.
- A scene can be created, edited at a basic level, saved, loaded, and rendered again.
- Editor camera/input behavior is usable without fighting ImGui interaction.
- Asset/model assignment exists at a basic level for mesh-renderer entities.
- The LearnOpenGL Phong-era renderer work is complete through shadow maps.
- PBR is explicitly deferred to a later milestone.

## Completed Foundation Slices

These are considered delivered and should not become new branches unless cleanup work is explicitly called out in an active deliverable set.

- [X] Engine owns the application loop, timing, active camera, active scene, and top-level editor mode.
- [X] Renderer global/singleton ownership was removed or hidden behind explicit renderer state.
- [X] Renderer frame input uses camera, viewport, renderables, and lights rather than gameplay state.
- [X] Renderer initialization accepts scene-derived configuration instead of hardcoded debug state.
- [X] Scene owns runtime entities and scene-level resources.
- [X] Default scene setup reproduces the original rendered output.
- [X] ECS supports entity IDs, component storage, names, transforms, mesh renderers, cameras, and light components.
- [X] ECS transforms, mesh renderers, and lights extract into renderer-friendly frame data.
- [X] Dear ImGui is selected and integrated through a small C++ adapter with a C-facing API.
- [X] Editor mode can be enabled/disabled around the renderer loop.
- [X] Editor panels exist for hierarchy, inspector, timing/status, scene actions, and camera settings.
- [X] Hierarchy lists ECS entities and supports selection.
- [X] Inspector displays and edits selected entity name and transform.
- [X] Inspector displays and edits selected light values.
- [X] Editor supports create empty, create renderable, duplicate renderable, and delete selected entity.
- [X] Editor/camera cursor mode toggle prevents UI interaction from always driving camera look.

## Deliverable Set 1: Editor Alpha Stabilization And Cleanup

Suggested branch: `cleanup-editor-alpha`

Goal: pay down the rough edges introduced while racing to commit 100 so future scene persistence and editor features do not pile onto a tangled `engine.c` loop.

Exit criteria:

- Commit-100 editor behavior still works after cleanup.
- Build is clean.
- Editor state gathering and action application are easier to read.
- Renderer no longer keeps obviously stale ownership copies for data now supplied per frame.

Tasks:

- [X] Remove stale renderer-owned light copies now that light data is passed through `RendererFrame`.
- [X] Make renderer upload functions accept `const` light/render data where they do not mutate inputs.
- [X] Normalize editor UI formatting and layout code after commit-100 iteration.
- [X] Extract repeated selected-entity/component gathering from `engine.c` into helper functions.
- [X] Extract editor action application paths from the main engine loop where practical.
- [X] Add small debug/assert helpers for invalid entity/component access.
- [X] Review fixed-size editor arrays and document current limits.
- [X] Revisit `imgui.ini` tracking policy.
- [X] Audit large assets and LFS tracking before the next asset-heavy commit.
- [X] Remove or quarantine old renderer/data types that are now legacy or unused.
- [X] Build a util for logging benchmarks on processes.
- [X] Add profiler logging for averages and abnormal frame/process spikes.
- [X] Take benchmarks on loops, heavy processes, and recursive functions.
- [X] Improve processes that have notable slowdowns or will become a constraint at a later time.
- [X] Add loaded and submitted renderer geometry stats to stats window.
- [ ] Expand profiler to track hardware utilization and log hardware spikes and potentially memory leaks.
- [ ] Apply Apache 2.0 License to the project.

Current fixed limits audit:

- Editor hierarchy display is capped at 256 entities per frame by `MAX_EDITOR_HIERARCHY_ITEMS` in `src/engine/engine.c`. Extra entities still exist in the ECS, but the hierarchy list is truncated. This is acceptable for the editor alpha and should be revisited when scene authoring grows beyond toy scenes.
- Render extraction is capped at 512 renderables per frame by `MAX_RENDERABLES` in `src/renderer/renderer_data.h`. `scene_extract_renderables(...)` stops adding renderables when that cap is reached. This is acceptable for now, but scene persistence should not treat 512 as a file-format limit.
- Point lights are capped at 4 by `MAX_SHADER_POINT_LIGHTS` in `src/renderer/data_types/lightObject.h` and `src/renderer/shaders/light.frag`. This is a shader/runtime renderer limit, not an ECS storage limit. Additional point light components may exist, but only the collection capacity can reach the renderer.
- Spot lights are capped at 4 by `MAX_SHADER_SPOT_LIGHTS` in `src/renderer/data_types/lightObject.h` and `src/renderer/shaders/light.frag`. Same current limitation as point lights.
- Default scene setup currently creates 4 active point lights and 2 active spot lights via `ACTIVE_POINT_LIGHTS` and `ACTIVE_SPOT_LIGHTS` in `src/scene/scene.c`. These are setup constants, not global engine limits.
- Entity names are capped at 64 bytes by `ENTITY_NAME_MAX_LENGTH` in `src/ecs/components.h`. Editor rename results are also capped at 64 bytes by `EDITOR_ENTITY_NAME_MAX_LENGTH` in `src/editor/editor_ui.h`. These currently match and should stay aligned until names move to dynamic storage.
- Skybox face paths are fixed at exactly 6 entries in scene, renderer config, and skybox loading APIs. This is an intentional cubemap shape, not a scalability problem.
- Small `float[3]` arrays in editor frame/result data are value-transfer buffers for vec3-like fields. They are not variable-capacity collections and do not need replacement for this milestone.
- ECS entity registry and component storage are dynamically grown from an initial capacity of 4. They are not fixed-size editor limits.
- Renderer/model helper buffers such as 512-byte texture/model paths and 64-byte shader uniform names are local implementation buffers. They are outside the editor-alpha fixed-array concern, but should be revisited during asset/persistence work if paths become user-authored data.

Follow-up candidates:

- Do not label geometry stats as visible until a real visibility/culling/GPU-statistics path exists. Current near-term stats should be named loaded or submitted geometry only.
- Add UI feedback when hierarchy or render extraction truncates data.
- Decide whether renderer light caps become shader defines, runtime config, or deferred clustered/forward-plus work.
- Keep scene file schema independent from current render/editor caps so saved scenes do not bake in alpha limitations.

## Deliverable Set 2: Scene Persistence V0

Suggested branch: `feat-scene-persistence-v0`

Goal: make the editor alpha capable of saving and loading reusable scene data without replacing the C-owned runtime scene model.

Exit criteria:

- Runtime scene data can be saved to disk.
- A saved scene can be loaded into C-owned scene/ECS state.
- Names, transforms, mesh renderer model paths, light values, and active camera state round-trip.
- The loaded scene renders successfully.
- JSON remains the default human-readable v0 format unless a documented decision changes it.

Tasks:

- [ ] Define v0 scene file schema for entities, names, transforms, mesh renderers, light values, and active camera.
- [ ] Add scene save path from current runtime ECS scene to disk.
- [ ] Add scene load path from disk into C-owned runtime scene data.
- [ ] Add save/load round-trip validation for save -> load -> render.
- [ ] Add editor action for save/load once the runtime path works.
- [ ] Add basic scene switching or scene reload during development.

## Deliverable Set 3: Editor Camera And Input V1

Suggested branch: `feat-editor-camera-input-v1`

Goal: make editor navigation and UI interaction feel intentional instead of relying only on the commit-100 Tab toggle.

Exit criteria:

- Editor cursor mode and camera mode are explicit and understandable in the UI.
- Viewport camera movement requires viewport-focused interaction or an explicit capture gesture.
- Typing into ImGui fields does not move the camera.
- Existing non-editor camera movement still works.

Tasks:

- [X] Add editor/camera cursor mode toggle.
- [ ] Add viewport-focused camera controls using viewport hover/focus or right-mouse capture.
- [ ] Prevent keyboard movement while typing in ImGui text fields.
- [ ] Decide whether editor camera and game camera should be separate states.
- [ ] Add UI-visible camera/input mode indicators if current display is insufficient.
- [ ] Document manual camera/input test cases.

## Deliverable Set 4: Asset And Mesh Assignment V0

Suggested branch: `feat-editor-asset-assignment-v0`

Goal: let mesh-renderer entities choose from known assets without building a full asset browser or runtime glTF cache yet.

Exit criteria:

- MeshRenderer entities can display and change their model path from the editor.
- Assignment uses known/supported assets only.
- Renderer behavior remains within current model-loading constraints.
- The branch does not become a full asset database/import pipeline.

Tasks:

- [ ] Define a small known-asset list or manifest for editor assignment.
- [ ] Display selected entity `MeshRendererComponent` details in the inspector.
- [ ] Add model path/asset selection for mesh-renderer entities.
- [ ] Decide how assignment interacts with current single-loaded-model renderer limitation.
- [ ] Add validation for missing asset/model paths.

## Deliverable Set 5: Scene Editing Workflow V1

Suggested branch: `feat-editor-scene-workflow-v1`

Goal: improve common editing workflows after persistence exists: dirty state, protected deletes, component summaries, and multi-entity operations.

Exit criteria:

- Editor clearly communicates selected entity/component state.
- Scene modifications can mark the scene dirty.
- Destructive actions have clear rules.
- Multi-select/group transform editing is either implemented or explicitly deferred with design notes.

Tasks:

- [ ] Add selected entity/component summary in inspector.
- [ ] Add component presence indicators for Transform, MeshRenderer, Light, and Camera.
- [ ] Add scene dirty state and save confirmation behavior.
- [ ] Add safe delete rules for protected/default scene entities if needed.
- [ ] Add multi-select and group transform editing.
- [ ] Add undo/redo design note, even if implementation waits.
- [ ] Add editor milestone demo checklist/documentation.

## Deliverable Set 6: Viewport Selection And Transform Tools

Suggested branch: `feat-editor-viewport-tools-v1`

Goal: move beyond hierarchy-first editing toward direct viewport interaction.

Exit criteria:

- Viewport object picking exists or a deliberate alternative is documented.
- Transform tooling is usable enough to move selected objects without relying only on numeric inspector fields.
- Hierarchy-first selection remains available as fallback.

Tasks:

- [X] Add temporary hierarchy-first selection.
- [ ] Add viewport object picking.
- [ ] Add translate/rotate/scale gizmo support.
- [ ] Decide whether gizmos are custom, ImGuizmo-based, or deferred.
- [ ] Add manual tests for picking and transform tool behavior.

## Deliverable Set 7: Editor Play/Simulation Separation

Suggested branch: `feat-editor-play-simulation-v1`

Goal: separate editing state from simulation/update state so editing does not require gameplay-like scene updates forever.

Exit criteria:

- Editor mode and play/simulation mode responsibilities are defined.
- Scene update can be paused or gated during editing.
- Renderer still receives valid scene data in both modes.
- UI exposes the current mode.

Tasks:

- [ ] Define editor mode vs play/simulation mode responsibilities.
- [ ] Gate `scene_update` so editing can pause simulation.
- [ ] Add UI-visible editor/play state.
- [ ] Ensure renderer still receives scene data in both editor and play modes.

## Deliverable Set 8: Phong Shadow Maps

Suggested branch: `feat-phong-shadow-maps`

Goal: finish the pre-PBR LearnOpenGL Phong section before starting the PBR milestone.

Exit criteria:

- Directional shadow mapping works with the current loaded scene.
- Shadow map resources have clear lifecycle ownership.
- Debugging/validation path exists for the shadow map.
- PBR remains explicitly deferred.

Tasks:

- [ ] Resume at shadow maps.
- [ ] Implement directional shadow mapping.
- [ ] Add shadow map framebuffer/resource lifecycle.
- [ ] Add depth shader pass.
- [ ] Integrate shadows into the current Phong/Blinn-Phong shader path.
- [ ] Add debug view or toggle for the shadow map.
- [ ] Validate shadows with the current loaded model scene.
- [ ] Document what is complete and explicitly mark PBR as deferred.
- [ ] Add visible/cull-surviving renderer geometry stats after the renderer has a real visibility path.

## Later, Not This Milestone

- PBR materials and image-based lighting.
- Physics.
- Audio.
- Scripting.
- Networking.
- Distribution/build packaging.
- Advanced asset database/import pipeline.
- Odin scripting/custom component bridge.
- Zig/C++ tooling experiments, only if a concrete tool need appears.
