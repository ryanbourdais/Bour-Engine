# Current Handoff — 2026-09-16

Active branch: `feat/window-render-target-resizing`.

DS2, DS3, and DS3A are complete and merged to `main`. DS3B's renderer-resize core is implemented and manually validated on this branch:

- Scene and MSAA render targets resize their existing color and depth/stencil attachments only when framebuffer-pixel dimensions change.
- The camera projection updates from current non-zero framebuffer dimensions.
- Zero-sized framebuffers defer scene rendering safely.
- Interactive window resizing preserved aspect ratio and produced no crashes or framebuffer-completeness errors.

Current uncommitted source changes are limited to `src/renderer/data_types/renderTarget.c`, `src/renderer/data_types/renderTarget.h`, and `src/renderer/renderer.c`. `git diff --check` and `cmake --build src/build --parallel` pass.

Next commit checkpoint: commit the DS3B renderer core and these notes with `feat(renderer): resize scene render targets with framebuffer`.

Next branch: `feat/editor-layout-foundation-v0`. DS4A will use fixed left/right/bottom ImGui panes around a central Scene View. The resolved scene texture will be drawn with `ImGui::Image`, while renderer target dimensions come from the panel's content bounds converted from logical ImGui units to framebuffer pixels. Docking, saved layouts, tabs, and multi-window workflows remain deferred.

DS3B remains open for HiDPI/UI-coordinate confirmation, expanded window-manager validation, and focused resize diagnostics. Keep `notes/backlog.md` and Linear synchronized as those checks are verified.
