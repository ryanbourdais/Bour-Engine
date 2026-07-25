Rendering Engine: Paused at Shadow Maps

Moving camera and input updates out of renderer_render_frame().
Introducing EngineState.
Removing the renderer’s global singleton.
Reducing the renderer’s direct dependency on GLFWwindow.
Giving timing its own abstraction.