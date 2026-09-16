# Temporary Handoff — Resize Core And Editor Viewport Foundation

Date: 2026-09-16
Branch: `feat/window-render-target-resizing`

## Collaboration Rule

This is a learning-first repository. The user manually edits all source code. The assistant may inspect source and run builds/tests, explain one small checkpoint at a time, and edit documentation only when explicitly requested. Do not apply source patches unless the user gives explicit permission and names the exact source files.

## Completed This Branch

- `RenderTarget` and `MsaaRenderTarget` reallocate their attached storage in place when dimensions change and validate framebuffer completeness.
- `Renderer` detects target-size mismatch from framebuffer pixel dimensions, resizes only when necessary, and updates projection with the same dimensions.
- Rendering safely defers for a zero-sized framebuffer.
- Manual resize validation passed: repeated resize preserved perspective and produced no crash or framebuffer-completeness error.

## Next Checkpoint

Commit the current DS3B renderer core and notes with:

```text
feat(renderer): resize scene render targets with framebuffer
```

Then create `feat/editor-layout-foundation-v0` for DS4A. Use fixed panes around a central Scene View. The Scene View owns logical ImGui bounds; convert those bounds using ImGui's framebuffer scale before giving the renderer pixel dimensions. Display the resolved scene texture with `ImGui::Image`. Defer docking, persistent layouts, tabs, and multi-window support.

## Remaining DS3B Work

- Verify logical ImGui coordinates, framebuffer pixels, and display scale under HiDPI or fractional scaling.
- Expand the manual matrix for tiling, maximize/restore, fullscreen, and minimize/restore.
- Add focused diagnostics for framebuffer completeness, target dimensions, and unchanged-size no-op behavior.

## Useful Files

- `notes/backlog.md` — canonical DS3B and DS4A tasks.
- `src/renderer/data_types/renderTarget.c/.h` — resizeable target ownership.
- `src/renderer/renderer.c` — target resize gate and camera projection update.
- `src/editor/editor_ui.cpp` — current floating panels; first DS4A layout target.
- `src/engine/engine.c` — frame sequencing and current framebuffer-size query.
