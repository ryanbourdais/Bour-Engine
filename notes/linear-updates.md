# Linear Update Reference — historical snapshot

This file records the September 16 DS3B/DS4A synchronization state and is no longer the current source of truth. As of 2026-09-25, inspect Linear directly and use `notes/wip.md` plus `notes/first-game-mvp-schedule.md` for local DS9C and First Game MVP context.

---

# Linear Update Reference — 2026-09-16

Linear was unavailable from this chat. Use these verified updates when the board is available. Do not change priority, assignee, cycle, or target date without separate direction.

## RYA-9 — Deliverable Set 3: Engine-Owned Geometry

Set status to **Done**.

> DS3 completed and was merged to `main` in `111675c` on 2026-09-15. Final manual regression validation passed: built-in primitives and programmable planes could be created, inspected, duplicated, deleted, restarted, and saved/loaded repeatedly. Renderer statistics and GPU lifecycle checks also passed.

## RYA-11 — Deliverable Set 3A: Programmable Mesh Primitive V0

Set status to **Done**.

> DS3A completed as part of the DS3 merge in `111675c` on 2026-09-15. V0 supports generated programmable planes with scene-owned CPU data, renderer-owned GPU resources, editor create/inspect/duplicate/delete flows, submitted/resident statistics, and persisted plane definitions. Raw vertex/index/color edits remain runtime-only by design.

## Deliverable Set 3B — Window And Render Target Resizing V0

Find the existing DS3B issue by title; do not create a duplicate. Keep it **In Progress**.

> Renderer resize core merged to `main` through PR #1 (`ffbbf0b`), from commit `2c7a1ff` (`feat: resize scene render targets with framebuffer`). Scene and MSAA targets resize their existing attachments only when framebuffer pixels change, projection updates from the same non-zero dimensions, and zero-size frames defer scene rendering. `git diff --check` and `cmake --build src/build --parallel` passed. Manual repeated resize preserved aspect ratio without crashes or framebuffer-completeness errors.
>
> Remaining acceptance work: verify logical ImGui coordinates versus framebuffer pixels under HiDPI/fractional scaling; complete tiling, maximize/restore, fullscreen, and minimize/restore checks; add focused resize diagnostics. DS4A will establish a central Scene View panel, providing the shared viewport contract needed for the UI-scale work.

## Deliverable Set 4A — Editor Layout Foundation V0

Find the existing DS4A issue by title; create one only if search confirms none exists. Set it to the team’s active in-progress status.

> **Layout decision, 2026-09-16:** implement a fixed-pane editor first: hierarchy and scene actions on the left, inspector and stats on the right, optional bottom controls, and a central Scene View. The Scene View will draw the resolved renderer texture through `ImGui::Image`. Its logical content bounds will be converted with ImGui framebuffer scale before sizing renderer targets. This creates a shared viewport contract for rendering, camera input, picking, and transform tools. Docking, persistent layouts, tabs, multi-window workflows, and polished theming are deferred.
>
> Implementation branch: `feat/editor-layout-foundation-v0`, based on `main` at `ffbbf0b`.
