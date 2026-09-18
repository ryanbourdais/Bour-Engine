# Handoff — Editor Layout Foundation V0

Date: 2026-09-16
Active branch: `feat/editor-layout-foundation-v0`
Base: `main` at `ffbbf0b` (`Merge pull request #1 from ryanbourdais/feat/window-render-target-resizing`)

## Repository Rules

The user manually edits all source code. The assistant may inspect source, run diagnostics, and edit documentation when asked. Do not edit source or perform Git state changes. Before work, inspect the related backlog/Linear issue; Linear is unavailable in this chat, so use `notes/linear-updates.md` as the prepared update reference.

## Delivered And Merged

- DS2, DS3, and DS3A are complete. DS3/DS3A were merged in `111675c` and manually regression-validated on 2026-09-15.
- DS3B renderer-resize core was committed in `2c7a1ff` and merged by PR #1 in `ffbbf0b`.
- `RenderTarget` and `MsaaRenderTarget` resize their existing attachments only when framebuffer-pixel dimensions change, then validate completeness.
- Renderer target sizes and camera projection use current non-zero framebuffer dimensions. Zero-size frames defer scene rendering.
- `git diff --check` and `cmake --build src/build --parallel` passed before the DS3B merge.
- Manual resize validation passed: repeated resize preserved scene aspect ratio with no crash or framebuffer-completeness error.

## Current Deliverable: DS4A

The first-pass layout decision is fixed panes, not docking:

- Left pane: hierarchy and scene actions.
- Right pane: inspector and stats.
- Optional bottom pane: camera or future tool controls.
- Center: `Scene View`, containing the resolved scene texture through `ImGui::Image`.

The Scene View owns the authoritative viewport rectangle. Its logical ImGui content size must be multiplied by `ImGuiIO::DisplayFramebufferScale` before it is passed to the renderer as pixel dimensions. That same rectangle will later govern camera capture, picking, and transform tools.

Defer docking, persistent layouts, tabs, multi-window support, and theming polish.

## First Implementation Checkpoint

Read these files before proposing changes:

- `src/editor/editor_ui.cpp` — current independent floating windows and `editor_ui_begin_frame`.
- `src/editor/editor_ui.h` — editor frame/result API.
- `src/engine/engine.c` — calls `editor_ui_begin_frame` before `renderer_render_frame`, which permits using panel dimensions in the same frame.
- `src/renderer/renderer.c/.h` — currently resolves the scene target and draws a fullscreen quad to the default framebuffer.
- `src/renderer/data_types/renderTarget.c/.h` — current resizeable offscreen target ownership.

First define a small editor viewport result with logical bounds, framebuffer-pixel size, hover/focus state, and the resolved scene texture handle. Do not add docking. Keep renderer pixels separate from ImGui logical coordinates.

## Remaining DS3B Work

- Verify ImGui logical coordinates, framebuffer pixels, and display scale under HiDPI or fractional scaling.
- Expand manual checks for tiling, maximize/restore, fullscreen, and minimize/restore.
- Add focused resize diagnostics for completeness, dimensions, and unchanged-size no-op behavior.

These checks can be completed while DS4A establishes the viewport contract.
