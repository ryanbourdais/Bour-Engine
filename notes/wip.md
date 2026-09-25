# Current Work — 2026-09-25

DS9C / RYA-44 — Engine/Editor Boundary And Separate Builds V0 is complete. `bour_engine` is a reusable runtime library; `bour_game` and `bour_editor` build and run separately. The editor uses an opaque `EngineRuntime` through snapshot, command, input, and render-target APIs, while the CMake boundary guard rejects editor/ImGui leakage into runtime files.

Next implementation target: DS8 / RYA-17 — Viewport Selection And Transform Tools. DS9C previously blocked it; the new editor host is now the correct place for editor-owned picking and transform interaction. RYA-93 remains a separate prerequisite for DS6 only. See `notes/first-game-mvp-schedule.md` for the pre-game dependency map.
