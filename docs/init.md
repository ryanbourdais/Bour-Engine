## To Build and Run

Make sure the target machine has these installed first:

- A C compiler
- CMake 3.20 or newer
- GLFW 3 development package with CMake config support
- OpenGL development libraries for the platform

This repository builds from the `src/` directory because the top-level `CMakeLists.txt` lives there.

```sh
# configure the project
cmake -S ./src -B ./src/build

# build the executable
cmake --build ./src/build

# run the output
./src/build/bour_engine
```

Notes:

- `glad` is already vendored in `src/external/glad`, so no extra loader setup is needed.
- If CMake cannot find `glfw3`, install the development package for your OS or point CMake at your package manager's prefix.
