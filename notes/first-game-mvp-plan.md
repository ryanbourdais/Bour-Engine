# First Game MVP — Standalone Playable Release

Projection prepared 2026-09-07. This is a planning estimate, not a delivery guarantee. Canonical acceptance criteria live in `notes/backlog.md`.

The planning reference is [the First Game MVP schedule](first-game-mvp-schedule.md): a feature-based Gantt and acceptance tracker. Target a playable/rough-package gate by December 2026 and MVP completion by March 2027. Dates are provisional and must be reforecast from demonstrated feature completion.

## Finish Line

One small game with a complete start/play/objective/result/restart/exit loop, authored using Bour Engine and packaged through a repeatable project workflow. The distributed game must launch without the editor, repository, compiler, or developer working-directory assumptions. Target one desktop platform first; select the platform and exact game concept at the gameplay gate. Do not assume a genre, general physics engine, or hot-reload system is required.

The project began March 11, 2026. December 31, 2026 through March 31, 2027 corresponds to roughly 9–12 months of total project history, or 4–7 months remaining from this review.

## Historical Velocity

Counting convention: commits reachable from the active branch HEAD, including merges. `git rev-list --count HEAD` reports 125 at `efb1fb4`; the historical editor-alpha commit `6a1d44e` reports 100 with the same convention. First-parent HEAD count is 122, so do not silently mix those measures. Uncommitted primitive/documentation work is not counted.

Counts below use author dates (`git log --format=%as HEAD`) through September 7, including inactive calendar days. Author dates are preferable here because integration rebased several commits onto later committer dates, but neither date measures hours worked.

| Window | Commits | Elapsed days | Commits/week |
|---|---:|---:|---:|
| March 11–September 7 | 125 | 180 | 4.86 |
| June 13–September 7 | 92 | 86 | 7.49 |
| August 1–September 7 | 35 | 37 | 6.62 |

The original count-based forecast suggested early March 2027. That is historical planning context only: there is no remaining-commit quota or numbered-commit finish line. March 2027 remains the planning target, subject to feature and integration evidence.

Commit frequency shows continuity of work, not remaining feature effort. Commits include documentation, cleanup, merges, and varied implementation sizes. Use accepted behavior, unresolved dependencies, and elapsed subsystem work to judge progress.

Historical feature anchors in the roadmap: engine/scene ownership and ECS developed through late July; editor alpha landed August 3–7; stabilization followed August 8–11; scene persistence ran from mid-August through August 27 and still requires a lifetime correction. Those spans support multi-week subsystem stages, but gameplay and packaging are new territory and deserve more contingency than familiar renderer work.

## Stage Gates

| Gate | Working calendar | Acceptance |
|---|---|---|
| Reliable geometry and persistence | September–October | Correct the two known bugs; finish DS3 including DS3A and reliable scene round trips. |
| Authoring and rendering foundation | October–November | Resize-safe viewport/input, usable selection/transforms and assignment, directional shadows. |
| Game Scripting and Entity Behavior V0 | Proof in October; integration through December | Narrow C-facing API, one entity-attached behavior, create/update/destroy lifecycle, persistence of supported behavior configuration, safe Play/Stop/load ownership, and packaged language dependencies. Odin is the primary direction; Lua is a fallback decision, not a second required integration. |
| First playable and package proof | November–December | Choose the small game/platform, establish the playable loop and Edit/Play separation, and launch a rough package outside the repository. |
| MVP completion | January | Freeze feature scope and finish the selected game, UI/feedback, standalone execution, and repeatable packaging. |
| Integration and release | February–March | Correct regressions, validate clean-environment playthroughs, document and release. |

The scripting/behavior workstream exposes an uncertainty previously hidden inside gameplay integration. Begin its proof early and reassess the December gate from the result; overlapping bars represent shared priorities, not extra developer capacity. Reserve the final stage for integration and unknowns. Defer optional content and polish before weakening required ownership or standalone-launch criteria.

## Minimal New Work Packages

- [ ] Game definition: choose one level/arena-sized experience, required controls, objective, result state, restart behavior, and target platform.
- [ ] Game-code boundary: prove one game-owned update behavior through a small C-facing engine API; evaluate the intended Odin integration and document any simpler fallback if build/debug friction threatens the schedule.
- [ ] Edit/Play and standalone runtime: separate authoring from simulation, restore editing state on Stop, and complete DS9C's separate runtime/editor targets so a selected game scene launches without editor UI, editor code, or Dear ImGui.
- [ ] Gameplay: implement only the movement, collision/triggers, UI, and optional audio necessary for the chosen loop.
- [ ] Package: include executable, required runtime dependencies, scene/shaders/assets, notices, and launch instructions through a repeatable build/package command. No source-tree absolute paths or dev-only assets.
- [ ] Release validation: launch from outside the repository in a clean target environment; play to the result, restart, exit, and repeat. Check missing-file errors and the absence of editor/toolchain requirements.

Engine dependencies such as supported graphics drivers remain platform prerequisites; standalone does not imply bundling an operating system or supporting every desktop platform.

## Reforecast Rule

After each stage, record completed behavior, actual calendar span, unresolved blockers, and the next acceptance test. Update dates from demonstrated feature completion. Keep commit 100 as project history; First Game MVP is complete when the playable distributable meets its acceptance criteria, regardless of commit count.
