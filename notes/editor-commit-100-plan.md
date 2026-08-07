# Editor By Commit 100 Plan

Commit 90 is the baseline already on the remote. Any commit after the planning reset counts toward the commit-100 editor alpha budget.

This plan is intentionally linked to [backlog.md](backlog.md). If the backlog changes, this file should change with it so we do not drift into interesting-but-off-milestone work.

## Commit 100 Target

By commit 100, Bour Engine should have a simple hierarchy-first editor alpha:

- the engine can launch with the editor UI enabled or disabled;
- Dear ImGui is initialized and shut down through a small C-friendly adapter;
- the editor has visible panels for hierarchy, inspector, and timing/status output;
- ECS entities can be listed, selected, inspected, and edited through the UI;
- transform edits made in the inspector affect what the renderer shows;
- at least one editor interaction beyond display is present, such as renaming or creating an entity;
- light data can be inspected or edited if the scene/light extraction path stays stable;
- the final commit leaves the alpha demo buildable and explainable.

This does not need to be a polished editor. It should prove the architecture: editor UI -> engine/scene/ECS state -> renderer input data -> viewport result.

## Mouse Behavior Warning

The current mouse behavior is not editor-safe. FPS-style cursor callbacks that always drive camera look are nonfunctional once the user can click editor panels, drag fields, or interact with a hierarchy.

Before transform editing becomes a real editor workflow, mouse input must be gated so UI interaction does not move the camera. The commit-100 plan handles this as an explicit milestone item:

- ImGui mouse capture should block camera mouse-look behavior.
- Viewport camera movement should require viewport focus, hover, or an explicit capture gesture such as holding right mouse.
- The editor should be allowed to consume mouse input without the renderer/game camera also responding.

Commit 97 is the planned point to change mouse behavior, because that is when transform editing becomes interactive enough for the current behavior to get in the way.

## Branch Rules

- Stay on the current ECS/editor integration branch only for work that belongs to the commit-100 editor alpha.
- Do not continue expanding the old scene/ECS foundation branch after it has been integrated into `main`.
- Use a dedicated editor branch, currently `feat-editor-commit-100`, for UI/editor-facing commits.
- If a task is not needed for the commit-100 editor alpha, move it back to [backlog.md](backlog.md) instead of spending one of the remaining commits on it.

## Commit Map

### Commit 91: Transform update smoke path

Goal: prove ECS transform changes can reach the renderer.

Acceptance:

- A transform can be changed outside hardcoded renderer setup.
- The visible model moves/rotates/scales from that state.
- The renderer still consumes renderable scene data rather than owning gameplay state.

### Commit 92: Camera update ownership

Goal: make camera update ownership clear before UI work.

Acceptance:

- Camera state lives at the engine/editor-facing layer.
- Renderer receives camera data as input.
- Existing camera movement still works in non-editor mode.

### Commit 93: UI route and minimal lifecycle

Goal: select Dear ImGui and wire the thinnest possible lifecycle.

Acceptance:

- Dear ImGui path is documented as the active choice.
- UI init/new-frame/render/shutdown are owned outside core renderer logic.
- Renderer remains a rendering backend, not the UI owner.

### Commit 94: Editor mode entry/toggle

Goal: allow editor UI to be enabled without making every run an editor run.

Acceptance:

- Editor mode can be toggled or configured on/off.
- UI rendering is skipped cleanly when disabled.
- Non-editor rendering still behaves as before.

### Commit 95: Panels, timing, and read-only hierarchy list

Goal: make the editor shell feel real without requiring mutation yet.

Acceptance:

- Show basic editor panels: hierarchy, inspector, and timing/status output.
- Display frame timing/FPS in UI.
- List ECS entities by name when possible, with a safe fallback label when unnamed.
- It is acceptable for the hierarchy to be read-only in this commit.

### Commit 96: Hierarchy selection and transform inspector display

Goal: connect editor selection to ECS data.

Acceptance:

- Clicking a hierarchy row stores a selected entity.
- Inspector displays selected entity name and transform data.
- Empty selection and deleted/invalid entities are handled safely.
- No transform editing is required yet.

### Commit 97: Editor mouse behavior gate and transform editing

Goal: make editor interaction usable and prove UI edits can drive scene state.

Acceptance:

- ImGui mouse capture blocks camera mouse look.
- Viewport camera behavior only runs when editor rules allow it, such as viewport focus/hover or explicit right-mouse capture.
- Inspector transform fields can edit position/rotation/scale.
- Renderer output updates from the edited ECS transform.

### Commit 98: Runtime entity creation, duplication, and rename

Goal: make the editor capable of instantiating and modifying runtime ECS entities without adding arbitrary asset loading yet.

Acceptance:

- Inspector can rename the selected entity.
- Hierarchy updates to show the new name.
- A create-empty action adds a new entity with at least `NameComponent` and `TransformComponent`.
- A create-renderable action adds a new entity with `NameComponent`, `TransformComponent`, and `MeshRendererComponent`.
- Newly created entities become selected immediately.
- Renderable entities appear in the viewport by reusing already-supported/default model data.
- If duplication is included, duplicating a selected renderable copies the selected transform and mesh-renderer data, offsets the new transform slightly, assigns a distinct name, and selects the duplicate.
- This commit does not add arbitrary model picking, asset browsing, runtime glTF resource loading, or a renderer-side model cache.

### Commit 99: Selected light inspection/editing

Goal: expose scene lighting through the editor path.

Acceptance:

- Selecting a light-backed entity shows relevant light fields.
- At least one simple light value can be edited if the extraction path supports it cleanly.
- Renderer light input still comes from scene/ECS extraction, not renderer-owned debug state.

### Commit 100: Delete selected entity if safe, commit alpha asset, then stabilization

Goal: close the v0.1 editor alpha with the remaining small scene interaction, the planned asset commit, and a coherent stabilization pass.

Acceptance:

- If ECS/component cleanup is straightforward, deleting the selected entity works safely.
- If deletion exposes unsafe ownership or lifecycle gaps, defer deletion and document the gap instead of forcing it.
- The planned commit-100 asset addition is included only if it is intentional, LFS-safe, and does not obscure the editor changes.
- Build is clean.
- Editor on/off behavior works.
- A short demo path exists: launch editor, view hierarchy, select entity, inspect/edit transform, create or duplicate a renderable entity, see viewport update, and verify the app exits cleanly.

## Explicitly Not In Commit 100

These are valid later tasks, but should not consume the commit-100 budget unless the plan is intentionally revised:

- scene save/load;
- scene switching or hot reload;
- asset browser;
- viewport picking;
- transform gizmos;
- docking polish;
- physics;
- scripting language bridge;
- full custom component authoring;
- shadow-map/editor lighting polish.

## Post-100 Direction

After commit 100, the next milestone should move from "editor exists" to "editor can author and persist scenes":

1. scene serialization and load/save;
2. editor-safe play/update separation;
3. viewport picking or gizmos;
4. asset/model assignment;
5. scripting/custom component spike, with Odin still the current front-runner for game-level customization.
