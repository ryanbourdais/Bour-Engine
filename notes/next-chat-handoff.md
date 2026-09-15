# Temporary Handoff — Primitive Persistence

Date: 2026-09-08  
Branch: `feat-engine-owned-geometry`

## Collaboration Rule

This is a learning-first repository. The user manually edits all source code. The assistant may inspect source and run builds/tests, explain one small checkpoint at a time, and edit documentation only when explicitly requested. Do not apply source patches unless the user gives explicit permission and names the exact source files.

## Completed This Branch

- Neutral `BuiltinPrimitiveType` values: cube, plane, quad, UV sphere, cylinder.
- Borrowed CPU primitive definitions in `primitive_mesh.c`; Renderer-owned GPU primitive resource collection.
- Plane winding corrected for `+Y` normals and CCW/front-face back-face culling.
- Renderer-owned opaque white primitive material/texture, primitive draw dispatch, loaded/submitted primitive statistics, and optional glTF startup loading.
- Primitive-only default scene is proven: cube, plane, quad, UV sphere, and cylinder render.
- UV sphere is fixed generated data: 16 segments, 8 rings, 153 vertices, 256 triangles.
- Cylinder is fixed generated data: 16 segments, 70 vertices, 64 triangles, with flat top/bottom cap normals.
- `scene/entity_factory.c/.h` now owns common asset/primitive entity assembly; default scene uses it.
- Loaded scene model/skybox path pointers are rebound after `*scene = loaded_scene`; repeated save/load validation is still pending.
- Documentation/backlog updated through primitive rendering and definitions.

## Current Persistence State

`test_scene.json` now saves all default primitives successfully. Primitive entries use stable string identifiers:

```json
"mesh_renderer": {
  "source_type": "primitive",
  "primitive_type": "cube",
  "material": null,
  "lod": null
}
```

Supported names are `cube`, `plane`, `quad`, `uv_sphere`, and `cylinder`.

The source-side writer lives in `src/scene/scene_serialization.c`:

- `builtin_primitive_type_name()` maps enum to stable string.
- `write_mesh_renderer_component()` handles `MESH_SOURCE_PRIMITIVE`.

Loading currently fails by design because `parse_mesh_renderer_component_v0()` supports only `"asset"`, and `ParsedEntityV0` stores only `mesh_model_path`. Earlier output was:

```text
Not yet implemented
Failed to save scene: 3
Failed to load scene: 4
```

That was before write support. Save failure `3` was `SCENE_SAVE_WRITE_FAILED`; it left partial JSON, leading to load failure `4` (`SCENE_LOAD_PARSE_FAILED`). Current saved output should be used for the next parse implementation.

## Immediate Next Checkpoint

Implement primitive parsing and runtime application, manually, in small steps:

1. Extend `ParsedEntityV0` in `src/scene/scene_serialization.h` with parsed mesh-source information:
   - `MeshSourceType mesh_source_type`
   - `BuiltinPrimitiveType mesh_primitive_type`
   - keep `mesh_model_path` for assets only.
2. Initialize those fields in the parsed-entity initializer.
3. Refactor `parse_mesh_renderer_component_v0()` to read `source_type` first:
   - `asset`: require and copy `model_path`.
   - `primitive`: require `primitive_type`; map the five stable strings back to enum values.
   - reject missing/unknown source or primitive names with `SCENE_LOAD_INVALID_SCENE`.
4. Update runtime apply so `MESH_SOURCE_ASSET` retains the owned-path flow, while `MESH_SOURCE_PRIMITIVE` creates a `MeshRendererComponent` with `model_path = NULL` and the parsed primitive enum.
5. Build, then Save → Load → Save → Load `test_scene.json`; verify all five hierarchy entities, transforms, and rendered primitives survive every round trip.

Do not add programmable mesh parsing yet. That is DS3A and has separate ownership/design work.

## Known Follow-up Work

- Repeated persistence validation, including asset model/skybox path lifetime.
- Editor Create Cube/Plane/Quad/etc. actions; the current generic `Create Renderable` test button is intentionally inert for the primitive-only default scene. A production asset workflow will use an asset browser/file picker.
- GPU allocation/upload error reporting and repeated renderer lifecycle validation.
- Retain only the license-free Supply Crate asset (`assets/models/supply_crate/supply_crate.gltf`) when the user finishes their manual asset cleanup. Documentation examples already use that path.
- DS3A programmable flat plane: ownership, safe modification API, dirty/update path, editor support, persistence.

## Useful Files

- `notes/backlog.md` — canonical DS3 tasks.
- `notes/initiative-backlog.md` — current high-level sequencing.
- `notes/wip.md` — current short handoff.
- `notes/scene-format-v0.md` — scene JSON contract; update it when primitive load support is complete.
- `src/scene/scene_serialization.c/.h` — current next implementation target.
- `src/scene/entity_factory.c/.h` — asset and primitive entity creation boundary.
