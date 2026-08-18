# Initiative Backlog

This file is the long-term initiative backlog for Bour Engine. It should stay higher-level than `notes/backlog.md`: use this file to keep the direction steady, and use `notes/backlog.md` for deliverable sets, branch-sized work, and task checklists.

## Planning Model

- `notes/initiative-backlog.md` tracks the engine's initiative-level north star.
- `notes/backlog.md` remains the canonical deliverable backlog for the active milestone and near-term branch work.
- Initiative entries should explain why the work matters, what unlocks it, and what done looks like.
- Deliverable sets should be moved into or out of `notes/backlog.md` as they become actionable.
- If a proposed task does not clearly advance an initiative, defer it or write down why it is still worth doing.

## Current North Star

Bour Engine is moving toward a C-first, editor-capable, scene-authored 3D engine where runtime ownership is explicit, saved scene data is reusable, renderer input comes from scene/ECS state, and higher-level gameplay/customization can eventually live above the core engine instead of inside renderer/debug scaffolding.

The near-term focus is still the Editor Foundation milestone: make scenes real, make the editor useful, finish the Phong-era renderer through shadows, and avoid pulling PBR, scripting, physics, networking, or packaging into the milestone before the foundation can hold them.

## Initiative 1: Runtime Ownership And Scene Truth

Status: active.

Purpose: make the engine own application state, scene state, entity data, active camera/skybox references, and update/render coordination without hidden renderer-owned demo state.

Why it matters:

- Scene persistence only becomes truthful when runtime scene data matches the saved format.
- Editor workflows need durable entity/component ownership, not one-off debug globals.
- Renderer features should consume frame data from the scene instead of becoming the place where game/editor state quietly accumulates.

Current deliverable anchors:

- Deliverable Set 2: Scene Persistence V0.
- Deliverable Set 6: Scene Editing Workflow V1.
- Deliverable Set 9: Editor Play/Simulation Separation.

Done signals:

- Default scene state is represented as real ECS-backed scene data.
- Active camera and active skybox are explicit scene-owned references.
- Save/load can round-trip useful scene data and render the loaded scene.
- Scene update can be gated or paused for editor workflows.
- Renderer state is resource/lifecycle ownership, not authoring truth.

## Initiative 1A: Component Metadata And Reflection

Status: planned.

Purpose: introduce a simple C-first metadata/reflection layer so built-in and future custom components can describe their fields, defaults, validation, editor exposure, and serialization rules without requiring fully bespoke serializers for every component type.

Why it matters:

- Scene Persistence V0 is already exposing repeated serializer work across transforms, renderers, cameras, skyboxes, lights, and future component blocks.
- Custom components and game-level scripting will need a general way to describe editable and serializable data.
- The editor inspector, scene serialization, validation, prefab-like workflows, and script/custom component bridges should eventually share component metadata instead of each inventing their own field model.

Current deliverable anchors:

- Deliverable Set 2: Scene Persistence V0.
- Future deliverable: Component Reflection And Generic Serialization V0.
- Later milestone: Odin scripting/custom component bridge.

Done signals:

- Components can register a stable type name, version, size, lifecycle hooks, and field descriptors.
- Field descriptors cover simple first-pass types such as bool, integer, unsigned integer, float, vec2/vec3/vec4, string/path, entity reference, and fixed arrays where needed.
- Descriptor-backed components can use generic JSON save/load for ordinary fields.
- Components with unusual ownership or runtime-resource behavior can still provide explicit override hooks.
- The editor inspector can reuse the same metadata for basic field display/editing instead of duplicating component-specific UI for every simple field.
- The reflection layer remains explicit, debuggable, and C-friendly rather than becoming hidden runtime magic.

## Initiative 2: Editor As A Real Authoring Tool

Status: active.

Purpose: turn the hierarchy-first editor alpha into a practical authoring surface for scenes, assets, transforms, lights, cameras, and common editing workflows.

Why it matters:

- The editor is the bridge between engine internals and usable game creation.
- Manual scene setup in C should become less necessary as scene data and editor workflows mature.
- Editor behavior needs documentation and test checklists so alpha limitations are visible instead of mysterious.

Current deliverable anchors:

- Deliverable Set 4: Editor Camera And Input V1.
- Deliverable Set 4A: Editor Layout Foundation V0.
- Deliverable Set 5: Asset And Mesh Assignment V0.
- Deliverable Set 6: Scene Editing Workflow V1.
- Deliverable Set 7: Editor User Documentation V0.
- Deliverable Set 8: Viewport Selection And Transform Tools.

Done signals:

- Camera/input behavior does not fight ImGui interaction.
- Users can create, select, inspect, modify, duplicate, delete, save, and load scene entities.
- Mesh renderer entities can choose supported assets or engine-owned geometry.
- Viewport selection and transform tooling can handle basic object placement.
- Editor-facing branches update user docs and manual test notes.

## Initiative 2A: Editor Workspace And Viewport UX

Status: active.

Purpose: evolve the editor from overlay panels on top of a rendered window into a cleaner workspace where the game/scene view lives inside a dedicated viewport, with tool windows arranged around it like standard professional game editors.

Why it matters:

- Many upcoming editor features depend on a trustworthy viewport boundary: camera capture, picking, transform tools, asset assignment previews, scene workflow feedback, and documentation screenshots.
- Overlay-only UI is useful for early debug visibility but becomes noisy once the editor is expected to author scenes.
- A contained viewport gives rendering, input, editor focus, and future tools a shared spatial contract.

Current deliverable anchors:

- Deliverable Set 4: Editor Camera And Input V1.
- Deliverable Set 4A: Editor Layout Foundation V0.
- Deliverable Set 8: Viewport Selection And Transform Tools.
- Deliverable Set 7: Editor User Documentation V0.

Done signals:

- The rendered game/scene view is treated as an editor viewport with explicit bounds.
- Hierarchy, inspector, stats, scene actions, camera settings, and future tool panels live around the viewport instead of competing with it.
- Viewport focus/capture rules are clear enough for camera control, picking, and gizmos.
- The first layout can stay simple, but the design leaves room for docking, saved layouts, tabs, multi-window editing, and polished editor theming later.
- UI layout changes are reflected in editor documentation and manual test checklists.

## Initiative 3: Engine-Owned Geometry And Procedural Content

Status: planned.

Purpose: reduce dependence on third-party test assets by giving the engine first-class primitives, programmable mesh data, SDF-generated meshes, and eventually terrain.

Why it matters:

- Public distribution should not depend on unclear or dev-only asset licensing.
- Procedural and engine-owned geometry gives the editor something native to create, save, load, and render.
- SDF and terrain work should grow from the same ownership and render-budget model, not as isolated experiments.

Current deliverable anchors:

- Deliverable Set 3: Engine-Owned Primitive And Programmable Geometry.
- Deliverable Set 3A: Initial SDF Generated Meshes.
- Deliverable Set 11: Level Of Detail And Render Budget Controls.
- Deliverable Set 11A: Advanced SDF Rendering And Workflows.
- Deliverable Set 12: Terrain Entity And Procedural Terrain.

Done signals:

- Scenes can use built-in primitives without external model files.
- Programmable meshes have explicit CPU and renderer-resource ownership rules.
- Initial SDF support produces ordinary generated mesh data before any advanced renderer path.
- LOD/render-budget controls exist before terrain-scale geometry arrives.
- Terrain has a first-class entity/component model that fits persistence, editor inspection, and render submission.

## Initiative 4: Renderer Capability Without Renderer Sprawl

Status: active.

Purpose: finish the pre-PBR renderer foundation while keeping rendering responsibilities bounded around resources, passes, shaders, and frame submission.

Why it matters:

- Shadow maps complete the current LearnOpenGL Phong-era target.
- Render stats, debug views, and resource lifecycles make later optimization work less guessy.
- PBR and advanced SDF rendering should start after the renderer boundary is stable enough to absorb them cleanly.

Current deliverable anchors:

- Deliverable Set 10: Phong Shadow Maps.
- Deliverable Set 11: Level Of Detail And Render Budget Controls.
- Later milestone: PBR materials and image-based lighting.
- Future deliverable after PBR: Flat Profiling And Optimization Pass.

Done signals:

- Directional shadow mapping works with clear framebuffer/depth-pass ownership.
- Renderer debug views or toggles expose the data needed to validate passes.
- Geometry stats describe loaded, submitted, and eventually visible/cull-surviving data accurately.
- Post-PBR profiling can compare engine, scene, editor, renderer, GPU-capable render timing, and present costs in one flat view before optimization work is chosen.
- PBR remains deferred until scene/editor/render foundations are complete.

## Initiative 5: Asset Pipeline And Distribution Hygiene

Status: planned.

Purpose: make asset usage, asset assignment, licensing, and future import tooling intentional enough that the project can be shared without dragging local-only test content into the engine identity.

Why it matters:

- Engine-owned content and clean licensing support public releases.
- Asset assignment should start small before becoming a full database or import pipeline.
- Future tooling can live outside core runtime if file processing and validation become complex.

Current deliverable anchors:

- Deliverable Set 3: Engine-Owned Primitive And Programmable Geometry.
- Deliverable Set 5: Asset And Mesh Assignment V0.
- Later milestone: Advanced asset database/import pipeline.
- `notes/language-trials.md`: asset import and processing candidates.

Done signals:

- Known asset assignment works for mesh-renderer entities.
- Missing or unsupported asset paths are validated clearly.
- Dev-only third-party assets are documented and separable from distributable project assets.
- Any future asset import tooling has a narrow file/tool boundary before it affects runtime architecture.

## Initiative 6: Language And Customization Boundaries

Status: watchlist.

Purpose: preserve the C-first engine core while leaving room for Odin game-level code, scripting-like customization, custom components, and small Zig/C++ tools where they solve a real boundary problem.

Why it matters:

- Language trials should inform architecture without turning the engine into a rewrite project.
- Game-level customization belongs above the core runtime once scene/editor foundations exist.
- Tooling languages are most valuable at clean file, validation, import, or editor-helper boundaries.

Current deliverable anchors:

- `notes/language-trials.md`.
- Later milestone: Odin scripting/custom component bridge.
- Later milestone: Zig/C++ tooling experiments, only when a concrete tool need appears.

Done signals:

- Engine/runtime architecture remains C-first.
- Language trials stay behind plain C-facing boundaries.
- Odin is evaluated for game-level customization when the engine has enough scene/component structure to make that meaningful.
- Zig/C++ tools are chosen for specific file/tooling problems, not as general rewrites.

## Initiative 7: Future Runtime Systems

Status: deferred.

Purpose: track the major game-engine systems that matter long term but should not crowd the Editor Foundation milestone.

Why it matters:

- Physics, audio, scripting, networking, UI, AI, and packaging are real engine goals.
- Pulling them in too early would weaken the current foundation work.
- Keeping them visible avoids losing the broader engine ambition while still protecting near-term focus.

Current deliverable anchors:

- `notes/core-expectations.md`.
- Later, Not This Milestone in `notes/backlog.md`.

Done signals:

- Each future system gets a milestone only after scene, editor, renderer, and asset ownership can support it.
- Each system starts with a narrow vertical slice and explicit integration boundary.
- Distribution/build packaging becomes a first-class initiative after the engine can produce reusable scenes and project-owned content.

## Sequencing Guardrails

- Finish truthful scene save/load before expanding editor workflows that depend on persisted state.
- Establish the editor layout foundation before adding viewport-heavy work such as picking, transform gizmos, asset previews, or richer scene workflow panels.
- Capture serialization boilerplate and field-metadata pain during Scene Persistence V0, but do not let reflection block the first truthful save/load path.
- Prefer engine-owned primitives before adding asset-heavy features.
- Keep initial SDF as generated mesh data before exploring raymarching, GPU volumes, or sculpting.
- Add LOD/render-budget controls before terrain-scale procedural geometry.
- Finish Phong shadow maps before beginning PBR.
- Keep docs current when editor behavior changes.
- Keep language trials small, bounded, and tied to actual subsystem pressure.
- Do not promote future systems into the active backlog until they have a narrow first slice and a foundation dependency has cleared.

## Immediate Planning Next Steps

- Keep Deliverable Set 2 as the next execution focus until save/load is truthful.
- When Deliverable Set 2 is complete, prioritize the editor layout foundation if viewport/input/UI structure is blocking the next usable scene-authoring demo.
- Review this initiative backlog whenever a new deliverable set is added, completed, or re-ordered in `notes/backlog.md`.
- If an item starts accumulating many branch-sized tasks here, move those tasks into `notes/backlog.md` and leave only the initiative intent and done signals in this file.
