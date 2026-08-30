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
- The editor has a cleaner layout where the rendered game/scene view lives inside an editor viewport area instead of all UI floating over the main rendered window.
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
- [X] Apply Apache 2.0 License to the project.

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

- [X] Define v0 scene file schema for entities, names, transforms, mesh renderers, light values, and active camera.
- [X] Add scene save path from current runtime ECS scene to disk.
- [X] Add scene load path from disk into C-owned runtime scene data.
- [X] Add save/load round-trip validation for save -> load -> render.
- [X] Add editor action for save/load once the runtime path works.
- [X] Add basic current-scene path tracking for development save/load.
- [X] Note repeated serializer boilerplate and candidate component-metadata/reflection needs discovered during V0 implementation.

## Deliverable Set 3: Engine-Owned Primitive And Programmable Geometry

Suggested branch: `feat-engine-owned-geometry`

Goal: reduce dependence on third-party test assets by giving the editor/scene system first-class engine-owned geometry that can be created, edited, saved, loaded, and rendered without external model files.

Exit criteria:

- Editor can create basic primitive entities without external model files.
- Runtime scene data can represent mesh source type: external asset, built-in primitive, or programmable mesh.
- Built-in primitives render through the same renderer submission path as asset-backed renderables.
- Programmable mesh entities have explicit ownership rules for generated vertex/index data or generated mesh resources.
- Scene persistence can round-trip primitive and programmable mesh entity definitions.
- The default/demo scene can rely primarily on primitives or project-owned geometry instead of third-party test assets.

Tasks:

- [X] Define mesh source/component model for asset-backed, primitive, and programmable mesh renderables.
- [X] Add built-in primitive definitions for cube, plane, and quad.
- [ ] Add renderer path for built-in primitive mesh resources without requiring glTF files.
- [ ] Add editor create actions for primitive entities.
- [ ] Add programmable mesh entity/component with explicit ownership rules for generated vertex/index data.
- [ ] Add save/load schema support for primitive and programmable mesh entities.
- [ ] Add sphere or UV sphere and cylinder primitive definitions before closing this deliverable.
- [ ] Replace current third-party test-scene dependency with primitives or project-owned geometry before public distribution.
- [ ] Document which third-party test assets remain local/dev-only and which engine-owned assets are safe to distribute.

## Deliverable Set 3A: Programmable Mesh Primitive V0

Suggested branch: `feat-programmable-mesh-primitive-v0`

Goal: create the first user-code-modifiable programmable mesh primitive: a flat plane by default, with vertices and colors that can be generated or modified in code while still flowing through normal scene, renderer, editor, and persistence paths.

Exit criteria:

- Runtime scene data can represent a programmable mesh primitive source that defaults to a flat plane.
- The default plane has explicit vertex, index, normal or normal-generation, UV if useful, and per-vertex or per-mesh color data rules.
- Code can modify or regenerate the programmable mesh's vertices and color data without bypassing the engine-owned mesh ownership model from Deliverable Set 3.
- Modified programmable mesh data renders through the same renderer submission path as other engine-owned geometry.
- Scene persistence can save/load the programmable mesh primitive's stable definition and any supported editable parameters.
- Regeneration and renderer-resource update rules are explicit when code changes the mesh data.
- SDF shader experiments, SDF mesh generation, terrain deformation, terrain chunking, collision, sculpting, and advanced material workflows are explicitly deferred.

Tasks:

- [ ] Define programmable mesh primitive data: vertex/index buffers, color data, optional UVs, normal strategy, dirty flags, and ownership rules.
- [ ] Add default flat-plane generation with stable dimensions, subdivisions if reasonable, and predictable winding.
- [ ] Add a code-facing API for modifying vertices and color data safely.
- [ ] Submit programmable mesh primitive data through the existing engine-owned geometry renderer path.
- [ ] Add editor create/inspect path for the programmable mesh primitive at a minimal level.
- [ ] Add save/load schema support for the programmable mesh primitive's stable definition and editable parameters.
- [ ] Document how this primitive can later feed SDF shader experiments, SDF-generated meshes, and terrain prototypes without committing to those systems now.

## Deliverable Set 4: Editor Camera And Input V1

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

## Deliverable Set 4A: Editor Layout Foundation V0

Suggested branch: `feat-editor-layout-foundation-v0`

Goal: add a short-term first pass at a cleaner editor layout so upcoming editor work has a stable UI foundation instead of continuing to stack overlay panels directly on top of the rendered window.

Exit criteria:

- The editor has a stable first-pass layout with a dedicated game/scene viewport area and predictable surrounding panels.
- Existing panels such as hierarchy, inspector, stats, scene actions, and camera settings are arranged around the viewport instead of freely covering the whole render surface by default.
- The viewport has explicit bounds that can be used by rendering, camera, input, picking, and future transform tools.
- Editor camera controls are scoped to the viewport region and do not rely on whole-window interaction.
- The first pass can use fixed or simple manually arranged panels; full docking, saved layouts, tabbed panels, multi-monitor workflows, and polished theming are deferred.
- Existing overlay/debug behavior remains available only where it is intentionally useful.

Tasks:

- [ ] Decide first layout model: fixed left/right/bottom panels, simple manually arranged ImGui windows, or an ImGui docking prototype.
- [ ] Define editor viewport bounds and feed them into input focus/capture decisions.
- [ ] Decide whether rendered scene output stays as the main framebuffer temporarily or moves to a framebuffer texture drawn inside an ImGui viewport panel.
- [ ] Arrange hierarchy, inspector, stats, scene actions, and camera settings around the viewport.
- [ ] Update camera/input behavior so viewport interaction drives camera movement only when the viewport is focused or captured.
- [ ] Make the layout good enough for viewport picking, transform tools, asset assignment, scene workflow, and editor documentation to build against.
- [ ] Document deferred editor layout work: full docking, saved user layouts, tabbed panels, multi-viewport/multi-window support, and polished editor theming.

## Deliverable Set 5: Asset And Mesh Assignment V0

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

## Deliverable Set 6: Scene Editing Workflow V1

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

## Deliverable Set 7: Editor User Documentation V0

Suggested branch: `docs-editor-user-guide-v0`

Goal: create practical editor documentation that can be used alongside development, manual testing, and future feature planning.

Exit criteria:

- Editor controls and modes are documented from a user/developer perspective.
- Current editor panels are explained with screenshots or clear text descriptions where useful.
- Common workflows have step-by-step documentation: create, select, duplicate, delete, edit transform, edit lights, save/load scene once persistence exists.
- Known limitations are documented so alpha behavior does not look like accidental breakage.
- Manual test checklists link back to the editor guide where appropriate.
- Documentation stays close enough to implementation that it can be updated during each editor-facing branch.

Tasks:

- [ ] Create `notes/editor-user-guide.md` or `docs/editor-user-guide.md`.
- [ ] Document editor vs camera mouse modes and current key/button controls.
- [ ] Document hierarchy, stats, camera settings, inspector, and scene actions panels.
- [ ] Document current entity workflows: create empty, create renderable, select, rename, duplicate, delete.
- [ ] Document transform and light editing workflows.
- [ ] Add scene persistence workflow docs after save/load exists.
- [ ] Add known limitations section for alpha editor behavior.
- [ ] Add a rule that editor-facing branches update this guide when behavior changes.

## Deliverable Set 8: Viewport Selection And Transform Tools

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

## Deliverable Set 9: Editor Play/Simulation Separation

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

## Deliverable Set 10: Phong Shadow Maps

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

## Deliverable Set 11: Level Of Detail And Render Budget Controls

Suggested branch: `feat-lod-render-budget`

Goal: add the first explicit LOD/resource-scaling controls before terrain introduces large generated meshes.

Exit criteria:

- Renderer/scene data can represent basic LOD policy for renderable entities.
- At least one mesh or primitive path can select between lower/higher detail geometry.
- Editor exposes minimal LOD/debug controls so behavior is visible while testing.
- Profiling/stats can show whether submitted geometry changes as LOD settings change.
- Terrain deliverable has a clear budget model to build against instead of inventing one mid-feature.

Tasks:

- [ ] Define first-pass LOD policy data: distance thresholds, forced LOD/debug override, and fallback behavior.
- [ ] Add renderer-side selection between LOD mesh levels for supported renderables.
- [ ] Add primitive/programmatic geometry support for generating lower-detail variants where reasonable.
- [ ] Add editor display/control for selected entity LOD state.
- [ ] Add submitted geometry stats that make LOD impact visible during testing.
- [ ] Document deferred LOD work: screen-space error, hysteresis, streaming, chunk LOD, impostors, and GPU-driven culling.

## Deliverable Set 11A: Advanced SDF Rendering And Workflows

Suggested branch: `feat-sdf-advanced-v1`

Goal: revisit signed distance fields after programmable mesh primitives and explicit LOD/render-budget controls exist, so heavier SDF workflows can be evaluated without quietly exploding geometry or shader cost.

Exit criteria:

- The engine has a documented decision on whether advanced SDF support should remain mesh-generated, add raymarched rendering, or support both paths.
- SDF experiments can build on the programmable mesh primitive path, participate in the LOD/render-budget model from Deliverable Set 11, or deliberately choose a separate raymarched/GPU-backed path with documented tradeoffs.
- At least one advanced SDF feature is proven in a contained prototype: boolean composition, smooth blending, cached LOD generation, raymarched rendering, or GPU-backed SDF data.
- Editor UI exposes enough controls to inspect advanced SDF behavior without hiding regeneration/rendering cost.
- Profiling or stats make SDF CPU generation cost, submitted geometry, or shader cost visible enough for iteration.
- Terrain-scale SDF use remains optional and can feed into Deliverable Set 12 without blocking it.

Tasks:

- [ ] Decide advanced SDF direction: generated meshes only, raymarched rendering, GPU SDF textures/volumes, or a hybrid.
- [ ] Integrate SDF mesh generation with LOD policy and submitted-geometry stats where applicable.
- [ ] Prototype one advanced SDF composition feature, such as union/subtract/intersect or smooth blending.
- [ ] Decide caching/invalidation rules for expensive SDF generation and renderer resource updates.
- [ ] Add editor debug controls for SDF cost, LOD, regeneration, and selected advanced parameters.
- [ ] Document constraints for SDF materials, lighting, shadows, collision, terrain, and runtime editing before expanding scope.

## Deliverable Set 12: Terrain Entity And Procedural Terrain

Suggested branch: `feat-terrain-entity`

Goal: introduce terrain as a first-class scene/rendering concept after the engine has enough LOD/render-budget groundwork to avoid exploding geometry cost.

Exit criteria:

- Terrain entities have a clear component/data model with dimensions, resolution, and height data strategy.
- Renderer can draw at least one terrain entity through a dedicated or reusable mesh path.
- Editor can create and inspect a terrain entity at a minimal level.
- Scene persistence can round-trip terrain definitions.
- The implementation uses or fits into the LOD/render-budget model from Deliverable Set 11.
- The implementation leaves room for heightmaps, procedural generation, chunking, collision, and streaming without requiring all of them in the first pass.

Tasks:

- [ ] Define terrain entity/component data model and ownership rules.
- [ ] Decide first terrain source: flat grid, generated height function, or heightmap-backed grid.
- [ ] Add minimal terrain mesh generation path.
- [ ] Add renderer submission path for terrain entities.
- [ ] Add editor create/inspect path for terrain entities.
- [ ] Add save/load schema support for terrain entities.
- [ ] Document deferred terrain work: heightmap import, sculpting, chunking, material painting, collision, and streaming.

## Later, Not This Milestone

- Hardware/system diagnostics profiler expansion: track CPU/GPU/memory utilization where platform support is clear, log hardware spikes, and investigate memory-leak tracking with an intentional debug-allocation strategy.
- PBR materials and image-based lighting.
- Post-PBR flat profiler, logging, and optimization pass.
- Physics.
- Audio.
- Scripting.
- Networking.
- Distribution/build packaging.
- Blank default scene, scene file viewing, and project asset management workflow.
- Advanced asset database/import pipeline.
- Odin scripting/custom component bridge.
- Zig/C++ tooling experiments, only if a concrete tool need appears.

## Future Deliverable: Blank Scene And Project File Browser V0

Suggested branch: `feat-editor-project-files-v0`

Goal: move from always booting a populated demo/default scene toward a real editor/project workflow: start from a truly blank scene, browse project files, open scene files, and manage known scene/assets from inside the editor without turning this into a full asset database yet.

Exit criteria:

- The editor can create or open a truly blank scene that does not auto-populate demo geometry, lights, or asset-backed test content.
- Demo/example scenes are explicit files or commands rather than hidden default runtime setup.
- The editor has a basic project/file view that can list known scene files and supported asset locations.
- Users can open an existing scene file from the editor file view once scene loading is stable.
- Users can see supported assets well enough to assign or reference them from scene/editor workflows.
- Missing, unsupported, or dev-only assets are surfaced clearly instead of silently failing or becoming public-distribution assumptions.
- The implementation remains a file/project browser and simple asset-management workflow, not a full import pipeline, dependency graph, content database, or packaging system.

Tasks:

- [ ] Define what a truly blank scene means for required active camera, skybox, lighting, and renderer fallback behavior.
- [ ] Decide how demo/example scenes are stored and launched separately from blank editor startup.
- [ ] Add a simple editor project/file view for scene files and supported asset directories.
- [ ] Add editor open-scene behavior from the file view after Scene Persistence V0 load/apply is stable.
- [ ] Connect the file view to known asset assignment without requiring a full asset database.
- [ ] Add validation and visible status for missing, unsupported, or dev-only asset references.
- [ ] Document deferred work: asset import, thumbnails/previews, dependency graphs, hot reload, packaging, project templates, and content database indexing.

## Future Deliverable After PBR: Flat Profiling And Optimization Pass

Suggested branch: `perf-flat-profiler-post-pbr`

Goal: after PBR is implemented and the renderer has absorbed its new passes, materials, resources, and state transitions, add a true flat profiler/logging pass and use it to clean up the worst accumulated CPU/GPU/editor/scene performance dirtiness.

Exit criteria:

- A flat profiler can report named engine/editor/scene/renderer processes in one comparable view instead of only isolated timers.
- Profiling logs capture per-frame process cost, averages, spikes, and enough context to identify recurring bottlenecks.
- Renderer-side timing has a clear path for GPU timing where platform/OpenGL support is available, while CPU-only fallback remains useful.
- Known current hot spots are measured before optimization, including scene extraction, linear ECS/component lookups, renderer frame stats, transparent mesh sorting/allocation, render submission, editor frame building, and present/swap timing.
- At least the highest-impact measured bottlenecks are optimized or explicitly deferred with notes explaining why.
- Optimizations preserve subsystem ownership boundaries instead of turning the profiler pass into an unbounded rewrite.
- The pass documents which later work should use multithreading/parallel jobs and which should first be fixed through better data layout, caching, batching, or resource ownership.

Tasks:

- [ ] Design a flat profiler view/model that can collect named process timings across engine, scene, editor, renderer, and present.
- [ ] Add structured profiler logging for frame index, process name, last cost, average cost, min/max, spike threshold, and relevant scene/render counts.
- [ ] Add or document a GPU timer query path for renderer passes where OpenGL support is available.
- [ ] Measure post-PBR frame cost in representative scenes before changing optimization-sensitive code.
- [ ] Measure and classify current known candidates: ECS lookup cost, scene extraction, renderer stats recomputation, transparent mesh allocation/sorting, render submission, editor frame assembly, and serialization/editor-triggered stalls.
- [ ] Optimize data-layout/caching issues before reaching for threads: entity-to-component indexing, reusable scratch buffers, cached renderer/model stats, batched render extraction, and model/resource lookup.
- [ ] Identify work that is genuinely parallel-friendly: asset loading, scene load/save, CPU generated geometry, SDF meshing, terrain/LOD generation, and future import processing.
- [ ] Add before/after profiler notes for each optimization that lands.
- [ ] Document deferred performance work that should wait for larger systems such as asset streaming, render graph/pass scheduling, job system, or GPU-driven culling.
