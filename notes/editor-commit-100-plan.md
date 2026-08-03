# Editor By Commit 100 Plan

Commit 90 marks the point where the engine/renderer/scene/ECS split became real enough to aim at an editor instead of continuing open-ended architecture cleanup.

The target for commit 100 is not a polished editor. The target is a visible, running editor shell that proves the engine can expose scene data to tools while the renderer remains a consumer of extracted render data.

Use this plan together with `notes/backlog.md`: the backlog is the source of truth for milestone scope, and this file is the commit-91-through-100 execution path. If the two disagree, update both before continuing.

## Commit 100 Target

By commit 100, Bour Engine should launch into a commit-100 v0.1 alpha editor with:

- the existing rendered viewport still working;
- a minimal UI layer initialized, rendered, and shut down cleanly;
- a simple hierarchy view listing ECS entities by name;
- hierarchy selection that persists across frames;
- a simple inspector view showing the selected entity's transform;
- editable transform values that update ECS data and affect rendering;
- basic selected-light inspection or editing if the selected entity has a light component;
- selected entity name display/rename through the inspector;
- a short alpha stabilization pass that verifies the editor loop still works after the pieces are connected;
- renderer, scene, ECS, and editor/UI responsibilities kept separate enough to continue safely.

If scope pressure appears, the fallback target is a naive editor shell:

- UI draws on top of or beside the viewport;
- hierarchy is read-only;
- inspector is read-only;
- transform editing is deferred to commit 101+.

## Baseline At Commit 90

Commit 90 is the editor-foundation baseline:

- engine owns the application loop;
- scene owns default entities/components;
- renderer consumes scene-derived render data;
- ECS renderables are submitted per frame;
- stale init-time renderer model-matrix state has been removed.

That means commits 91 through 100 should stop extending the renderer/scene split and start turning the split into visible editor tooling.

## Branch And Commit Rules

Commit 90 is already on the remote branch. Any new commit after this plan is commit 91 or later.

Current branch: `feat-scene-ecs-foundation`. Use it only for the remaining ECS/system proof work that naturally belongs to the current branch. After commit 91, or earlier if the ECS smoke path is skipped, create a new editor-focused branch before starting UI integration. Suggested branch name:

```text
feat-editor-commit-100
```

Branch switch rule:

- stay on `feat-scene-ecs-foundation` for commit 91 if it is the transform update smoke path;
- switch to `feat-editor-commit-100` before the Dear ImGui/UI route decision or any editor lifecycle code;
- do not mix large asset/LFS commits into the editor branch unless the editor task explicitly requires them.

## Commit Budget

There are ten commits from 91 through 100. The plan assumes each commit should be a small, reviewable slice.

Commits 93 and 94 are intentionally combined because the UI route decision should immediately prove itself with a blank UI lifecycle. Commits 97 and 98 from the first draft are also combined because listing and selecting hierarchy entities are one coherent feature.

The two recovered commits are used for light inspector support and selected-entity rename support. Commit 100 still includes an alpha stabilization pass, but stabilization is part of the acceptance criteria rather than the whole commit.

### Commit 91: add a transform update smoke path

Goal: prove ECS transform changes flow through per-frame render extraction into rendering.

Acceptance:

- a transform can be changed during scene update or through a small debug helper;
- rendered model matrix reflects the ECS transform without renderer knowing about ECS;
- temporary/debug behavior is clearly marked.

Backlog item:

- `Add a simple transform update path for ECS transform-derived render data.`

### Commit 92: clarify camera update ownership

Goal: keep camera behavior outside the renderer and document the bridge to future ECS/editor camera work.

Acceptance:

- camera update remains engine-owned or is wrapped by a small engine-side helper;
- renderer receives camera data only through `RendererFrame`;
- backlog records whether ECS camera bridging is now or later.

Backlog items:

- `Add a simple camera update path that keeps camera behavior outside the renderer.`
- `Decide whether camera should remain engine-owned or become an ECS camera component bridge.`

### Commit 93: choose UI route and add minimal UI lifecycle

Goal: make the UI implementation choice and prove it by rendering a blank UI frame.

Acceptance:

- choose Dear ImGui unless a concrete blocker appears;
- decide whether integration uses a C-friendly boundary or a small C++ adapter;
- UI init happens after window/OpenGL setup;
- UI frame begins/ends inside the engine loop;
- UI shutdown happens during engine shutdown;
- viewport rendering still works;
- the decision is captured in backlog or a short note.

Backlog items:

- `Choose immediate-mode UI path, likely Dear ImGui unless there is a strong reason not to.`
- `If Dear ImGui is selected, evaluate whether the UI boundary should remain C-friendly or use a small C++ adapter.`
- `Add UI initialization/shutdown around the existing GLFW/OpenGL loop.`

### Commit 94: add editor mode entry or toggle

Goal: make it explicit when the runtime is operating as an editor.

Acceptance:

- editor mode can be enabled by default, compile-time flag, launch flag, or simple runtime toggle;
- editor code path is visible in engine coordination code;
- game/runtime camera behavior is not permanently tangled with editor behavior.

Backlog item:

- `Add editor mode toggle or editor executable entry path.`

### Commit 95: add first editor panels and timing output

Goal: get visible editor structure on screen.

Acceptance:

- hierarchy panel exists;
- inspector panel exists;
- FPS/frame timing moves into UI or is duplicated there;
- panels may be simple/non-dockable if docking setup is too much for the commit budget.

Backlog items:

- `Add simple panels: viewport, hierarchy, inspector, and timing/log output. Docking can wait if it slows commit 100.`
- `Display FPS/frame timing in UI instead of only the window title.`

### Commit 96: list and select ECS entities in hierarchy

Goal: expose scene/ECS data to the editor UI and establish selected-entity state.

Acceptance:

- hierarchy lists live entities;
- names are shown when `NameComponent` exists;
- unnamed entities have a stable fallback label;
- clicking a hierarchy entry stores the selected entity;
- selected state survives across frames;
- invalid/deleted entity selection has a safe fallback;
- inspector can query the selected entity;
- no renderer dependency is introduced into ECS or scene data.

Backlog item:

- `Add object selection from the hierarchy.`

### Commit 97: show selected transform in inspector

Goal: inspect selected entity transform data.

Acceptance:

- inspector shows transform position, rotation, and scale when present;
- inspector has a clear empty state when no entity or no transform is selected;
- transform display reads ECS data directly or through a small scene/editor query helper.

Backlog item:

- `Add inspector display for selected entity transform.`

### Commit 98: edit transform and see renderer update

Goal: cross from read-only inspection to a naive but real editor loop.

Acceptance:

- inspector can edit transform position, rotation, or scale;
- ECS transform changes are visible in the viewport;
- renderer still consumes per-frame extracted draw data;
- demo is: select entity, change transform, see scene update.

Backlog item:

- `Add inspector editing for transform values.`

### Commit 99: inspect or edit selected light values

Goal: make the inspector useful for more than transforms by exposing existing ECS light components.

Acceptance:

- inspector detects directional, point, or spot light components on the selected entity;
- inspector displays core light fields such as color, direction, position, attenuation, or cutoff values as applicable;
- editing at least one simple light value is preferred, but read-only display is acceptable if UI integration risk is high;
- renderer continues consuming extracted light data rather than editor-owned light state.

Backlog item:

- `Add inspector display/editing for light values.`

### Commit 100: rename selected entity and stabilize v0.1 alpha

Goal: add one more small editor feature, then make the commit-100 demo coherent, reproducible, and safe to continue from.

Acceptance:

- inspector displays the selected entity name when a `NameComponent` exists;
- inspector can rename the selected entity through the `NameComponent`;
- hierarchy reflects renamed entities;
- build is clean;
- app launches into editor-capable mode;
- viewport still renders;
- hierarchy selection works;
- selected transform display works;
- transform edit affects rendering;
- light inspection does not break rendering;
- backlog accurately marks completed commit-100 items;
- any deferred editor work is explicitly left for commit 101+.

Backlog item:

- `Add selected entity name display/rename in the inspector.`

Stabilization pass:

- run the commit-100 demo checklist before marking the alpha complete.

## Not In Commit 100

These are important, but they are not required for the commit-100 editor target:

- scene save/load;
- scene switching/reload;
- asset browser;
- gizmos;
- object picking from viewport;
- physics;
- scripting;
- shadow maps;
- polished docking layout.

## Guiding Principle

Commit 100 should prove the editor loop:

```text
ECS scene data -> editor UI -> edited component -> render extraction -> renderer frame -> viewport
```

If that loop exists, even in a naive form, the editor milestone has crossed from architecture prep into usable tooling.
