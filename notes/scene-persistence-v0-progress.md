# Scene Persistence V0 Progress Handoff

Current pause point: scene load parser foundation is in progress for Deliverable Set 2.

## Current status

The scene persistence deliverable has crossed from schema/save work into load parsing. The save path already writes the current runtime ECS scene to JSON, and the load path now has a parser foundation backed by `jsmn`.

Implemented so far:

- JSON parser dependency added through `src/external/jsmn`.
- Parsed scene/entity intermediate types added.
- Scene load entry point reads scene JSON from disk.
- Top-level scene validation handles:
  - `version`
  - `active_camera.entity`
  - `active_skybox.entity`
  - `entities`
  - duplicate entity id rejection
  - active camera/skybox entity existence
- Parsed entity data currently covers:
  - entity id
  - optional name
  - required transform
  - optional camera
  - optional skybox
- Active camera now validates that the referenced entity has a parsed camera component.
- Active skybox now validates that the referenced entity has a parsed skybox component.
- Skybox face paths are copied into fixed parsed buffers instead of pointing into the temporary JSON buffer.
- Parser failure cleanup is centralized in `parse_scene_json`; helper functions return status rather than freeing parent-owned parser state.

## Important ownership notes

The parsed scene is still an intermediate representation. It does not yet rebuild or mutate the live runtime `Scene`.

String ownership remains one of the main design points for the runtime apply step:

- Parsed skybox paths are currently safe because they live in fixed buffers on `ParsedEntityV0`.
- Mesh renderer paths should follow the same parsed-buffer pattern next.
- Runtime `MeshRendererComponent` and `SkyboxComponent` currently store `const char *`, so applying parsed data into runtime ECS needs a clear ownership strategy before those pointers can be trusted after load.

## What is still left for Deliverable Set 2

Backlog exit criteria still require:

- A saved scene can be loaded into C-owned scene/ECS state.
- Names, transforms, mesh renderer model paths, light values, and active camera state round-trip.
- The loaded scene renders successfully.
- Editor save/load actions exist once the runtime path works.
- Basic scene switching or reload exists during development.
- Serializer boilerplate/reflection notes are captured.

## Recommended next slices

1. Parse mesh renderer next.

   Add parsed fields for:

   - `has_mesh_renderer`
   - copied `mesh_model_path`

   Parse:

   - required `model_path`
   - optional/reserved `source_type`, accepting `"file"` for V0
   - ignore `material` and `lod` for now

2. Start runtime apply after mesh renderer.

   Add an internal function shaped like:

   ```c
   static SceneLoadResult apply_parsed_scene_to_runtime(Scene *scene, const ParsedSceneV0 *parsed);
   ```

   First target should apply:

   - preserved entity ids
   - names
   - transforms
   - cameras
   - skybox
   - mesh renderers
   - active camera
   - active skybox

3. Add light parsing/apply after the first loaded scene renders.

   Remaining component parsers:

   - `directional_light`
   - `point_light`
   - `spot_light`

4. Validate round-trip behavior.

   Manual V0 validation is enough initially:

   - save current scene
   - load saved scene
   - confirm rendered model, transform, camera, skybox, and lights survive

5. Add editor actions.

   Only after runtime load works:

   - Save Scene
   - Load Scene
   - Reload Scene

## Current risk areas

- Preserving entity ids may require ECS/entity-registry support that does not exist yet.
- Runtime string ownership may need scene-owned copied strings or a small asset/path storage strategy.
- Load failure should not leave the runtime scene half-mutated.
- Parser boilerplate is already repetitive; note this for the reflection/component-metadata backlog item, but do not block V0 on solving reflection.

