# Project Assets V0

## Purpose

This document defines the minimum project-asset contract for Bour Engine's
first game. It gives scenes and runtime systems one portable way to refer to
project-owned files without introducing an asset database, GUIDs, or an import
pipeline.

This is the contract for RYA-76. Packaging automation is RYA-77.

## Terms

- **Project root**: the directory selected when an editor or game runtime is
  launched. It owns the project scene files and project assets.
- **Asset reference**: a normalized, project-relative path serialized in a
  scene or other project-owned data.
- **Resolved filesystem path**: the runtime-only path formed by joining a
  project root with an asset reference. It is never serialized.
- **Engine resource**: a file distributed with the engine/runtime itself, such
  as a renderer shader. Engine resources are not project assets and are not
  selected from a project's `assets/` directory.

## Project Layout V0

The supported project layout is intentionally small:

```text
project-root/
  test_scene.json
  assets/
    models/
    cubemaps/
    ...
```

`assets/` is the conventional home for project-owned source assets. The
runtime does not yet require a manifest, asset database, or import cache.

## Serialized Reference Rule

All supported project-owned references are normalized paths relative to the
project root. Examples:

```text
test_scene.json
assets/models/supply_crate/supply_crate.gltf
assets/cubemaps/skybox/right.jpg
```

References must use `/` separators and must not contain:

- absolute paths;
- `.` or `..` path segments;
- empty path segments, including trailing or doubled `/`;
- backslashes; or
- `:` characters, including drive prefixes.

The runtime validates a reference before filesystem I/O. It joins only a valid
reference with the selected project root. A resolved filesystem path must not
be written back into scene data.

## Current Supported Uses

The rule currently applies to:

- the initial scene path supplied to `EngineRuntimeCreateInfo`;
- editor Save Scene and Load Scene commands;
- mesh-renderer glTF model paths; and
- skybox cubemap face paths.

The editor accepts an optional project-root argument:

```text
bour_editor [project-root]
```

The standalone game accepts an explicit project root and renderer shader
directory:

```text
bour_game [--project-root <path>] [--renderer-shader-directory <path>] [scene-path]
```

When no root is supplied, the runtime uses the current directory for
compatibility. This fallback is intended for repository development, not as a
package-layout rule.

## Runtime and Cache Behavior

The model cache uses the serialized project-relative reference as its cache
key. It resolves a separate filesystem path only when loading the resource.
This keeps scene persistence portable while allowing the same project to be
opened from different locations.

Failed model loads remain cached as failed entries for the lifetime of the
renderer. A bad reference therefore produces an actionable error once rather
than once per rendered frame. Missing or invalid skybox references fail
renderer initialization with an error identifying the offending face. Failed
initial scene loads report both the requested scene reference and project root.

## Engine Resources

Renderer shaders are engine-owned resources. They must not be represented by
project-relative asset references or loaded from a project's `assets/`
directory. The runtime accepts a renderer shader directory independently of
the project root. The development fallback is `src/renderer/shaders`; a
standalone launch supplies its packaged shader directory explicitly.

## Packaging Boundary

RYA-76 defines the layout and reference rule. RYA-77 is responsible for a
repeatable command that copies the game executable, runtime dependencies,
engine resources, selected startup scene, and required project assets into a
standalone package.

The minimum intended package distinction is:

```text
package/
  bour_game
  engine_resources/
    shaders/
  project/
    startup_scene.json
    assets/
```

The packaged game must receive `project/` as its project root. Asset discovery,
dependency graphs, GUIDs, reimporting, hot reload, thumbnails, and a general
asset browser remain deferred.

For the V0 package layout, the game launch contract is:

```text
bour_game \
  --project-root project \
  --renderer-shader-directory engine_resources/shaders \
  startup_scene.json
```

Copying the bounded `project/assets/` directory satisfies V0 asset inclusion.
RYA-77 will automate that copy and package assembly.

## Validation Checklist

Before considering this contract complete:

- [x] Open the editor with a copied temporary project root.
- [x] Confirm scene load, Save Scene, and Load Scene operate only in that root.
- [x] Confirm model assets and all six skybox faces load from that root.
- [x] Confirm Play preview uses the same project root.
- [x] Confirm malformed or missing scene/model/skybox references produce useful
  errors without undefined behavior or per-frame log spam.
- [x] Confirm `bour_game` accepts and uses an explicit project root.
- [x] Confirm engine shaders load from a copied engine-resource directory while
  launching outside the repository working directory.

## Verified 2026-10-27

A copied temporary project containing `test_scene.json`, `assets/`, and a
copied `engine_resources/shaders/` directory was launched from `/tmp` with
`bour_game`. Scene, model, skybox, and shader loading all resolved from the
explicit paths. The following negative cases were also verified:

- a missing initial scene names its relative reference and project root;
- a missing model reports once while other renderables continue loading; and
- a missing skybox face prevents renderer initialization and names the
  filesystem path that could not be opened.
