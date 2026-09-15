# Scene Format V0

Scene Format V0 is the first human-readable scene persistence format for Bour Engine. It is intentionally JSON so early editor saves can be inspected, diffed, and repaired by hand while the runtime scene/ECS model is still evolving.

The format describes scene data, not renderer internals. Loading a scene should rebuild C-owned runtime scene/ECS state from this file.

Reviewed 2026-09-14: asset, built-in primitive, and programmable-plane persistence, plus editor save/load, are implemented. Loaded asset model paths and skybox face paths are rebound to destination-owned scene storage. Repeated Save → Load → Save → Load validation passed with the Supply Crate asset, skybox, built-in primitives, transforms, lights, active camera, and a transformed 2×2 programmable plane.

## Design Rules

- Entity IDs are saved and restored. A loaded scene should preserve the entity IDs present in the file.
- The loader must update the runtime entity registry so future entities are allocated above the highest loaded ID.
- `active_camera.entity` is required in V0. Multiple inactive cameras are allowed, but one active camera must be declared.
- `active_skybox.entity` is required in V0. Skyboxes are represented through normal entity component data instead of root-level special-case renderer data.
- Entities may contain multiple component blocks directly. This mirrors the ECS model and leaves a natural path for custom script/component blocks later.
- Component blocks are optional per entity. Missing optional component blocks mean the entity simply does not have that component.
- Malformed required fields and duplicate IDs reject the whole load. Per-entity recovery is not implemented.
- Strings loaded from JSON must be copied into owned runtime storage before being attached to components. The loader must not store pointers into a temporary JSON buffer.
- Unrecognized fields are currently ignored without a general warning/preservation mechanism. Unsupported mesh source types are rejected.

## Top-Level Shape

```json
{
  "version": 0,
  "active_camera": {
    "entity": 1
  },
  "active_skybox": {
    "entity": 2
  },
  "entities": []
}
```

### `version`

Required integer. Must be `0` for this format.

### `active_camera.entity`

Required entity ID. The referenced entity must exist and must have a `camera` component.

### `active_skybox.entity`

Required entity ID. The referenced entity must exist and must have a `skybox` component.

### `entities`

Required array. Each object in the array describes one entity and any components attached to it.

## Entity Shape

```json
{
  "id": 10,
  "name": "Entity Name",
  "enabled": true,
  "transform": {},
  "mesh_renderer": {},
  "camera": {},
  "skybox": {},
  "directional_light": {},
  "point_light": {},
  "spot_light": {},
  "scripts": []
}
```

### `id`

Required unsigned integer. Entity IDs must be unique within the file and must be restored by the loader.

### `name`

Optional string. If omitted, the loader may assign an empty name or generated editor name. Current runtime name storage is bounded by `ENTITY_NAME_MAX_LENGTH`.

### `enabled`

Reserved optional boolean. V0 may write this field, but runtime behavior does not need to depend on it yet. If omitted, treat as `true`.

### Component blocks

All component blocks are optional unless referenced by required top-level state such as `active_camera` or `active_skybox`. Missing component blocks mean the entity does not have that component.

## Components

### Transform

```json
"transform": {
  "position": [0.0, 0.0, 0.0],
  "rotation": [0.0, 0.0, 0.0],
  "scale": [1.0, 1.0, 1.0]
}
```

All fields are optional within the transform block. Missing transform fields use the default transform values shown above.

### Mesh Renderer

```json
"mesh_renderer": {
  "source_type": "asset",
  "model_path": "assets/models/supply_crate/supply_crate.gltf",
  "material": null,
  "lod": null
}
```

`source_type` is required. V0 supports `asset`, `primitive`, and the generated programmable-plane definition below.

`model_path` is required when `source_type` is `asset`. The path should be relative to the project/runtime working directory. The loader must copy this string into owned runtime storage.

```json
"mesh_renderer": {
  "source_type": "primitive",
  "primitive_type": "cube",
  "material": null,
  "lod": null
}
```

`primitive_type` is required when `source_type` is `primitive`. Supported stable names are `cube`, `plane`, `quad`, `uv_sphere`, and `cylinder`. Primitive renderers have no `model_path`.

```json
"mesh_renderer": {
  "source_type": "programmable",
  "programmable_type": "plane",
  "width": 2.000,
  "depth": 2.000,
  "material": null,
  "lod": null
}
```

`programmable_type` is required when `source_type` is `programmable`. V0 accepts only `plane`; `width` and `depth` must be finite positive numbers. Loading regenerates a fresh scene-owned CPU mesh and a fresh runtime mesh ID from this definition. Raw vertex, index, and color edits made through the code-facing API are runtime-only in V0 and are not serialized. SDF generation, terrain generation, chunking, sculpting, collision, and advanced material workflows remain future work.

`material` and `lod` are reserved extension points and currently ignored without warnings.

### Camera

```json
"camera": {
  "fov": 45.0,
  "near_clip": 0.1,
  "far_clip": 100.0
}
```

A camera entity should usually also have a transform. The active camera is selected by the required top-level `active_camera.entity` reference.

### Skybox

```json
"skybox": {
  "faces": [
    "assets/cubemaps/skybox/right.jpg",
    "assets/cubemaps/skybox/left.jpg",
    "assets/cubemaps/skybox/top.jpg",
    "assets/cubemaps/skybox/bottom.jpg",
    "assets/cubemaps/skybox/front.jpg",
    "assets/cubemaps/skybox/back.jpg"
  ]
}
```

`faces` is required and must contain exactly six strings in renderer order: right, left, top, bottom, front, back. The loader must copy all face paths into owned runtime storage.

### Directional Light

```json
"directional_light": {
  "direction": [-0.2, -1.0, -0.3],
  "ambient": [0.02, 0.02, 0.02],
  "diffuse": [0.05, 0.05, 0.05],
  "specular": [0.02, 0.02, 0.02]
}
```

All fields are required when the `directional_light` block is present.

### Point Light

```json
"point_light": {
  "position": [0.0, 2.5, 0.0],
  "ambient": [0.0, 0.0, 0.0],
  "diffuse": [3.0, 0.2, 0.2],
  "specular": [3.0, 0.2, 0.2],
  "constant": 1.0,
  "linear": 0.09,
  "quadratic": 0.032,
  "has_visual": true
}
```

All fields except `has_visual` are required when the `point_light` block is present. If `has_visual` is omitted, treat it as `false`.

### Spot Light

```json
"spot_light": {
  "position": [0.0, 3.0, 2.0],
  "direction": [0.0, -1.0, -0.3],
  "ambient": [0.0, 0.0, 0.0],
  "diffuse": [4.0, 0.0, 2.5],
  "specular": [4.0, 0.0, 2.5],
  "constant": 1.0,
  "linear": 0.09,
  "quadratic": 0.032,
  "inner_cutoff_degrees": 25.0,
  "outer_cutoff_degrees": 45.0
}
```

All fields are required when the `spot_light` block is present.

### Scripts

```json
"scripts": [
  {
    "language": "odin",
    "module": "scripts/player_controller.odin",
    "component": "PlayerController",
    "properties": {}
  }
]
```

`scripts` is reserved for future custom component/game-level scripting work. V0 may preserve this field as data, but runtime script execution is not part of Scene Format V0.

## Complete Example

```json
{
  "version": 0,
  "active_camera": {
    "entity": 1
  },
  "active_skybox": {
    "entity": 2
  },
  "entities": [
    {
      "id": 1,
      "name": "Editor Camera",
      "transform": {
        "position": [0.0, 1.5, 6.0],
        "rotation": [0.0, 0.0, 0.0],
        "scale": [1.0, 1.0, 1.0]
      },
      "camera": {
        "fov": 45.0,
        "near_clip": 0.1,
        "far_clip": 100.0
      }
    },
    {
      "id": 2,
      "name": "Default Skybox",
      "skybox": {
        "faces": [
          "assets/cubemaps/skybox/right.jpg",
          "assets/cubemaps/skybox/left.jpg",
          "assets/cubemaps/skybox/top.jpg",
          "assets/cubemaps/skybox/bottom.jpg",
          "assets/cubemaps/skybox/front.jpg",
          "assets/cubemaps/skybox/back.jpg"
        ]
      }
    },
    {
      "id": 3,
      "name": "Supply Crate",
      "transform": {
        "position": [0.0, 0.0, 0.0],
        "rotation": [0.0, 0.0, 0.0],
        "scale": [1.0, 1.0, 1.0]
      },
      "mesh_renderer": {
        "source_type": "asset",
        "model_path": "assets/models/supply_crate/supply_crate.gltf",
        "material": null,
        "lod": null
      }
    },
    {
      "id": 4,
      "name": "Sun",
      "directional_light": {
        "direction": [-0.2, -1.0, -0.3],
        "ambient": [0.02, 0.02, 0.02],
        "diffuse": [0.05, 0.05, 0.05],
        "specular": [0.02, 0.02, 0.02]
      }
    },
    {
      "id": 5,
      "name": "Point Light 0",
      "point_light": {
        "position": [0.0, 2.5, 0.0],
        "ambient": [0.0, 0.0, 0.0],
        "diffuse": [3.0, 0.2, 0.2],
        "specular": [3.0, 0.2, 0.2],
        "constant": 1.0,
        "linear": 0.09,
        "quadratic": 0.032,
        "has_visual": true
      }
    },
    {
      "id": 6,
      "name": "Spot Light 0",
      "spot_light": {
        "position": [0.0, 3.0, 2.0],
        "direction": [0.0, -1.0, -0.3],
        "ambient": [0.0, 0.0, 0.0],
        "diffuse": [4.0, 0.0, 2.5],
        "specular": [4.0, 0.0, 2.5],
        "constant": 1.0,
        "linear": 0.09,
        "quadratic": 0.032,
        "inner_cutoff_degrees": 25.0,
        "outer_cutoff_degrees": 45.0
      }
    }
  ]
}
```

## Loader Behavior Notes

- Preserve entity IDs from the file.
- Reject duplicate entity IDs for the entire load.
- After loading, set the registry next ID above the highest valid loaded ID.
- Log malformed required fields with entity ID and component name when possible.
- Reject malformed entities rather than attempting partial recovery.
- Missing or invalid active camera/skybox references reject the load; automatic fallback is not implemented.
- Copy all loaded strings into runtime-owned storage.
- Unknown component blocks are ignored; warning and preservation support is not implemented.

## Future Version Hooks

Reserved fields in V0 exist to keep the file shape stable, not to imply full support. Likely future extensions include:

- programmable mesh variants beyond the generated plane, including serialized raw vertex/index/color buffers
- material assignment
- LOD policy
- terrain entities
- custom script components
- parent/child hierarchy
- editor-only metadata
- prefab/source asset references
