# Current Handoff — 2026-09-08

Active branch: `feat-engine-owned-geometry`. Canonical tasks: `notes/backlog.md`, DS3; DS3A details its programmable-mesh subtask.

Implemented and visually checked: neutral primitive identifiers; cube, plane, quad, UV-sphere, and cylinder definitions; scene draw-data translation; explicit mesh lifecycle; primitive resource collection; renderer-owned default material; draw dispatch; primitive statistics; and primitive-only renderer startup. Default-scene construction uses the scene entity factory.

Completed corrections:

1. Loaded component paths are rebound to destination-owned scene storage after runtime scene replacement; repeated persistence validation remains pending.
2. Plane indices now match `+Y` normals and the renderer's CCW front-face convention.

Persistence verification completed: built-in primitive and asset scene entries, including the Supply Crate and skybox, survived repeated Save → Load → Save → Load validation with hierarchy, transforms, lights, and active camera state intact. DS2 is complete.

Editor primitive creation completed: Cube, Plane, Quad, UV Sphere, and Cylinder actions create and select renderable entities; manual validation covered transforms, save/load, and the preserved Supply Crate asset action.

Asset policy completed: every model under `assets/models/` is user-approved; the unreferenced starter textures were removed.

Primitive lifecycle reliability completed: allocation/upload failures report OpenGL errors and clean up partial resources; three fresh startup/shutdown cycles rendered all primitives without upload errors.

DS3 and DS3A completed 2026-09-15. Final regression validation covered repeated editor create/inspect/duplicate/delete, fresh lifecycle runs without console or OpenGL errors, renderer statistics, and Save → Load → Save → Load for transformed programmable planes. The V0 persistence contract saves only the generated plane definition and dimensions; raw vertex/index/color edits are runtime-only.

Backlog synchronization: keep `notes/backlog.md` and `notes/initiative-backlog.md` aligned with Linear as scope and verified progress change. DS3 maps to RYA-9 and DS3A maps to RYA-11; both are ready to mark Done following the completed validation.

Engine coordinator, explicit renderer state, engine-owned camera updates, and timing abstraction are already implemented. Shadow maps remain a later renderer deliverable.
