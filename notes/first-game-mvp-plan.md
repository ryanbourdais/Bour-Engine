# First Game MVP — Standalone Playable Release

Originally projected 2026-09-07; materially revised 2026-09-25 after the Game 1 concept was selected and its supporting runtime work was decomposed in Linear. This is a planning estimate, not a delivery guarantee. Canonical acceptance criteria live in `notes/backlog.md` and the linked Linear issues.

The planning reference is [the First Game MVP schedule](first-game-mvp-schedule.md): a feature-based dependency map and acceptance tracker. The current Linear target for the MVP is June 15, 2027. Dates are provisional and must be reforecast from demonstrated feature completion.

## Finish Line

One compact 10–20 minute underwater exploration dive with a complete start/play/survey-completion/restart/exit loop. The player swims through one authored dive site, discovers and documents roughly 8–12 marine species plus 2–3 landmarks, encounters one memorable showcase creature, and completes the survey. The distributed game must launch without the editor, repository, compiler, or developer working-directory assumptions. Target one desktop platform first; platform selection remains an implementation decision.

Scope guards: no combat, survival systems, crafting, economy, boat gameplay, open-world ocean, procedural ecosystem, realistic dive simulation, broad quest framework, hundreds of species, generalized AI, or full animation-authoring/editor tooling.

## Historical Velocity

Counting convention: commits reachable from the active branch HEAD, including merges. `git rev-list --count HEAD` reports 125 at `efb1fb4`; the historical editor-alpha commit `6a1d44e` reports 100 with the same convention. First-parent HEAD count is 122, so do not silently mix those measures. Uncommitted primitive/documentation work is not counted.

Counts below use author dates (`git log --format=%as HEAD`) through September 7, including inactive calendar days. Author dates are preferable here because integration rebased several commits onto later committer dates, but neither date measures hours worked.

| Window | Commits | Elapsed days | Commits/week |
|---|---:|---:|---:|
| March 11–September 7 | 125 | 180 | 4.86 |
| June 13–September 7 | 92 | 86 | 7.49 |
| August 1–September 7 | 35 | 37 | 6.62 |

The original count-based forecast suggested early March 2027. That baseline is retired: it predated the selected underwater-dive concept and the explicitly scoped runtime dependencies. There is no remaining-commit quota or numbered-commit finish line. The June 15, 2027 Linear target remains subject to feature and integration evidence.

Commit frequency shows continuity of work, not remaining feature effort. Commits include documentation, cleanup, merges, and varied implementation sizes. Use accepted behavior, unresolved dependencies, and elapsed subsystem work to judge progress.

Historical feature anchors in the roadmap: engine/scene ownership and ECS developed through late July; editor alpha landed August 3–7; stabilization followed August 8–11; scene persistence ran from mid-August through August 27 and still requires a lifetime correction. Those spans support multi-week subsystem stages, but gameplay and packaging are new territory and deserve more contingency than familiar renderer work.

## Stage Gates

| Gate | Working calendar | Acceptance |
|---|---|---|
| Engine/editor boundary | September–October 2026 | Complete DS9C: independently buildable game/editor clients over the opaque runtime boundary, then validate the standalone game path. |
| Authoring and runtime foundations | October 2026–early 2027 | Deliver the bounded authoring, Edit/Play, collision, game-code, asset-reference, input, scene-lifecycle, UI, audio, animation, and underwater-rendering capabilities required by the dive. |
| Dive implementation | After the required foundations | Build the diver, discovery loop, wildlife behavior, site blockout, bounded asset set, survey UI, sound, and visual pass. |
| Package, playtest, and release | Final MVP stage | Produce a clean standalone package, complete fresh-launch playtests, correct blockers, and release the dive. |

The scripting/behavior workstream remains the largest integration uncertainty. Prove it before game content depends on it; Odin remains the primary direction and a bounded fallback is a decision, not a second simultaneous integration. Overlapping workstreams are dependency groupings, not parallel staffing. Reserve the final stage for integration and unknowns; defer optional content and polish before weakening required ownership or standalone-launch criteria.

## Minimal New Work Packages

- [X] Game definition: one compact underwater discovery dive, its bounded content envelope, objective, completion, restart behavior, and scope guards are selected. Target platform remains to be chosen.
- [ ] Game-code boundary: prove one game-owned update behavior through a small C-facing engine API; evaluate the intended Odin integration and document any simpler fallback if build/debug friction threatens the schedule.
- [ ] Edit/Play and standalone runtime: keep authoring state separate from an isolated runtime preview; Play starts that preview from authoring state, and Stop discards temporary preview edits. Complete DS9C's `bour_engine` library plus `bour_game`/`bour_editor` targets so a selected game scene launches without editor UI, editor code, or Dear ImGui. Apply-back behavior and in-editor build orchestration are deferred until the targets are established.
- [ ] Runner and launch boundary: runners own their windows, event loops, swapping, and presentation; the runtime renders only to a supplied target. Support independent `EngineRuntime` sessions, including editor off-screen textures, and give `bour_game` an explicit scene/project argument plus one documented development default.
- [ ] Gameplay: implement only underwater diver movement, discovery/survey interaction, bounded wildlife behavior, UI, audio, and collision/query support necessary for the dive.
- [ ] Package: include executable, required runtime dependencies, scenes/shaders/assets/scripts/configuration, notices, and launch instructions through a repeatable build/package command. No source-tree absolute paths or dev-only assets.
- [ ] Release validation: launch from outside the repository in a clean target environment; play to the result, restart, exit, and repeat. Check missing-file errors and the absence of editor/toolchain requirements.

Engine dependencies such as supported graphics drivers remain platform prerequisites; standalone does not imply bundling an operating system or supporting every desktop platform.

## Reforecast Rule

After each stage, record completed behavior, actual calendar span, unresolved blockers, and the next acceptance test. Update dates from demonstrated feature completion. Keep commit 100 as project history; First Game MVP is complete when the playable distributable meets its acceptance criteria, regardless of commit count.
