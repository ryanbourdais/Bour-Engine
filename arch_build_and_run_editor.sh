#!/usr/bin/env bash
clear

set -euo pipefail

project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${project_root}/src/build"

for dependency in cmake pkg-config; do
    if ! command -v "${dependency}" >/dev/null 2>&1; then
        echo "Missing dependency: ${dependency}" >&2
        echo "Install Arch dependencies with: sudo pacman -S --needed cmake glfw pkgconf git-lfs" >&2
        exit 1
    fi
done

if ! pkg-config --exists glfw3; then
    echo "Missing dependency: GLFW" >&2
    echo "Install Arch dependencies with: sudo pacman -S --needed cmake glfw pkgconf git-lfs" >&2
    exit 1
fi

if [[ ! -f "${project_root}/src/external/cglm/CMakeLists.txt" || \
      ! -f "${project_root}/src/external/imgui/imgui.cpp" ]]; then
    git -C "${project_root}" submodule update --init --recursive
fi

cmake -S "${project_root}/src" -B "${build_dir}" -DCMAKE_BUILD_TYPE=Debug
cmake --build "${build_dir}" --target bour_editor --parallel

cd "${project_root}"
exec "${build_dir}/bour_editor"
