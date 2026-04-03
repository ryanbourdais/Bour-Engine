#include "mesh.h"

// TODO(render-object/material): Once RenderObject exists, revisit this API.
// Texture creation should likely target a material/appearance layer rather than mutating Mesh directly.
int create_texture(Mesh *mesh, char* file_location);
