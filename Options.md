1. stencil-buffer render pass for object outlines, masking, portals, or editor selection.
2. Pre-loading textures so startup time is much faster
3. Framebuffers:
- Defer generic post-processing for now.
- Revisit for HDR/tone mapping, bloom, MSAA resolve, shadow maps, and picking.
4. Moving transforms into drawing objects to open the door more to an Update() loop
5. Volumetrics
6. Sprite system
7. Animations