# Language Trials

## Purpose

Explore Odin, C++, and Zig through small subsystem implementations before deciding whether any part of Bour Engine should be rewritten. These trials are learning exercises first and rewrite evaluations second.

The current editor milestone should remain focused on engine ownership, scene management, ECS, UI, editor interaction, and completing the Phong-era renderer through shadow maps. Language trials should only enter that milestone when they directly support one of those goals without delaying it.

Current direction: keep the engine core C-first. Odin is the front-runner for future game-level code, scripting-like behavior, custom components, and higher-level customization above the engine runtime. Zig remains a strong candidate for serialization, validation, asset tooling, and other file/tool-oriented boundaries.

## Trial Rules

- Keep each trial behind a plain C-facing boundary.
- Do not rewrite a working subsystem just to use a new language.
- Prefer leaf modules before core engine control flow.
- Keep the C implementation as the reference until the replacement proves itself.
- Measure integration friction, build complexity, debugger experience, memory ownership clarity, and API readability.
- A successful trial should make the engine easier to understand or extend, not just more novel.

## Best Candidates

### 1. Asset Import And Processing

Recommended language: Zig or C++.

Why it is a good trial:

- Asset import is important but naturally separable from runtime engine flow.
- It can be tested with files and command-line tools before being wired into the editor.
- It exercises strings, paths, allocation, file IO, binary parsing, and error reporting.
- The existing glTF loading path gives a concrete comparison point.

Candidate work:

- Build an offline asset inspection tool.
- Normalize model paths, texture references, and material metadata.
- Emit a simple intermediate asset manifest.
- Later, support editor import workflows.

Milestone fit: mostly out of scope for Editor Foundation unless limited to simple model-path validation for the asset panel.

### 2. Scene Serialization

Recommended language: Zig.

Why it is a good trial:

- Serialization is central to the editor but can stay separate from rendering.
- Zig is a strong fit for explicit allocation, small tools, and C ABI boundaries.
- The output format can remain simple and inspectable.

Candidate work:

- Serialize entity IDs, names, transforms, model paths, light settings, and active camera.
- Load a saved scene into C-owned runtime structures.
- Add round-trip tests: save, load, compare.

Milestone fit: in scope if kept minimal, because scene save/load is part of the editor foundation.

### 3. ECS Storage Prototype

Recommended language: Odin or Zig.

Why it is a good trial:

- ECS is data-oriented and small enough to prototype cleanly.
- Odin and Zig both encourage explicit data layout and low-level control.
- The subsystem has clear operations: create entity, attach component, query component, iterate components.

Candidate work:

- Prototype component storage outside the main engine first.
- Compare sparse-set, packed-array, and simple fixed-capacity approaches.
- Keep component definitions compatible with C structs.

Milestone fit: risky but possible. The main ECS should probably be written in C for the editor milestone, with Odin/Zig prototypes informing the design.

Current status: defer this as an engine-storage replacement trial. The current C component storage is simple enough that Odin does not yet provide a meaningful architectural win. Revisit Odin here when the problem becomes query ergonomics, script-authored components, editor-exposed component metadata, or higher-level gameplay systems rather than raw storage.

### 4. Math And Transform Utilities

Recommended language: Odin or Zig.

Why it is a good trial:

- Transform math is small, practical, and easy to verify.
- It lets you compare language ergonomics without pulling in renderer complexity.
- It can be benchmarked and tested independently.

Candidate work:

- Build transform composition/decomposition helpers.
- Generate model matrices from position, rotation, and scale.
- Validate output against cglm usage.

Milestone fit: in scope only as a prototype or reference. Avoid replacing cglm during Editor Foundation.

### 5. Editor Utility Tools

Recommended language: C++ or Zig.

Why it is a good trial:

- Editor tools are naturally adjacent to, but not always inside, the runtime.
- C++ is useful if Dear ImGui becomes the editor UI path.
- Zig is useful for small build-friendly utilities.

Candidate work:

- Asset manifest generator.
- Scene validator.
- Shader reload/index tool.
- Build/run helper for editor test scenes.

Milestone fit: partially in scope if the tool directly supports the editor workflow.

## Poor First Candidates

### Renderer Core

Avoid as an early language trial.

The renderer currently carries shader programs, framebuffers, loaded models, lights, camera UBOs, skybox state, and OpenGL state setup. Rewriting it while the engine/renderer boundary is still moving would mix two hard problems: architecture cleanup and language evaluation.

Revisit after:

- Renderer no longer owns scene data.
- Render input is a stable frame packet or render world.
- Shadow maps are complete.
- The editor viewport exists.

### Engine Loop

Avoid for now.

The engine loop is the coordination point for windowing, timing, input, camera update, rendering, and eventually scene/editor update. This is a bad place to add foreign-language build and ABI questions until the architecture settles.

### UI Layer

Avoid deciding too early.

If Dear ImGui is chosen, C++ may become useful at the UI boundary. But the first decision should be whether the editor uses Dear ImGui, not whether the editor is a C++ trial.

## Language Fit Notes

### Odin

Best for:

- Game-level coding above the C engine core.
- Scripting-like behavior and custom components.
- Data-oriented gameplay/system prototypes.
- Editor-exposed customization layers.
- ECS/query experiments once component metadata and higher-level behavior become real pain points.

Watch for:

- Build-system integration with the existing CMake project.
- C ABI comfort.
- Long-term library ecosystem needs.
- Avoiding accidental rewrites of stable C engine systems just because Odin is faster to iterate in.

### C++

Best for:

- Dear ImGui integration if the editor uses it directly.
- Tooling that benefits from mature libraries.
- Renderer-adjacent experiments after the renderer boundary stabilizes.

Watch for:

- Keeping the C engine boundary clean.
- Avoiding a gradual unplanned conversion of core engine code.
- Compile complexity and ownership semantics.

### Zig

Best for:

- Asset tools.
- Scene serialization.
- Build utilities.
- Explicit-memory prototypes with C ABI boundaries.

Watch for:

- Toolchain maturity and version churn.
- CMake/build integration choices.
- Whether Zig is used as a tool language, a subsystem language, or a rewrite candidate.

## Suggested Trial Order

1. Zig scene serialization spike.
2. Odin custom-component or game-level scripting spike after the C ECS has scene data and update boundaries.
3. C++ Dear ImGui/editor shell spike, only if Dear ImGui is selected.
4. Zig or C++ asset manifest/import tool.
5. Reassess after the editor can load, show, edit, and save a basic scene.

## Decision Checklist

After each trial, answer:

- Was the language pleasant for this subsystem?
- Was the C boundary simple?
- Did the build get meaningfully harder?
- Did debugging get better or worse?
- Did memory ownership become clearer?
- Would this subsystem be easier to maintain in this language after three months?
- Did this trial help the editor milestone, or distract from it?

## Parking Lot

- PBR renderer rewrite experiments.
- Physics subsystem language trial.
- Scripting language/runtime decisions, with Odin currently the leading candidate for game-level code and custom components.
- Full engine rewrite decision.
- Cross-platform packaging implications.
