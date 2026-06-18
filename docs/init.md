## Build and Run

This repository builds from the `src/` directory because the top-level `CMakeLists.txt` lives there.

## Initialize Git Submodules

The project uses `cglm` as a Git submodule. After cloning the repository, initialize it before configuring CMake:

```sh
git submodule update --init --recursive
```

For a fresh clone, you can initialize submodules automatically:

```sh
git clone --recurse-submodules <repository-url>
```

If this step is skipped, CMake will report that `src/external/cglm` does not contain a `CMakeLists.txt` file.

## macOS Startup

Make sure the machine has these installed first:

- A C compiler
- CMake 3.20 or newer
- GLFW 3 development package with CMake config support
- OpenGL development libraries for the platform

```sh
# configure the project
cmake -S ./src -B ./src/build

# build the executable
cmake --build ./src/build

# run the output
./src/build/bour_engine
```

## Windows Startup

Make sure the machine has these installed first:

- Visual Studio Build Tools or Visual Studio with the Desktop development with C++ workload
- CMake 3.20 or newer
- `vcpkg`
- GLFW 3 installed through `vcpkg`

If `vcpkg` is not bootstrapped yet:

```powershell
cd C:\vcpkg-2026.03.18
.\bootstrap-vcpkg.bat
```

Install GLFW:

```powershell
.\vcpkg install glfw3:x64-windows
```

Optional, but useful for VS Code and Visual Studio integration:

```powershell
.\vcpkg integrate install
```

Configure and build the project with the `vcpkg` toolchain:

```powershell
cmake -S .\src -B .\build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg-2026.03.18/scripts/buildsystems/vcpkg.cmake
cmake --build .\build
```

Run the executable:

```powershell
.\build\Debug\bour_engine.exe
```

Notes:

- `glad` is already vendored in `src/external/glad`, so no extra loader setup is needed.
- GLFW does not provide a `glfw.c` file like `glad`. It is a compiled library that CMake finds with `find_package(glfw3 CONFIG REQUIRED)`.
- If CMake cannot find `glfw3` on Windows, make sure the build directory was configured with the `vcpkg` toolchain file and delete the build directory before reconfiguring if needed.
