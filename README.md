# Bour Engine
A from-scratch game engine written in C, using OpenGL via GLFW and GLAD.

This project is focused on learning low-level graphics programming, engine architecture, and building systems without relying on heavy frameworks or AI-generated code.

## Goals
- Build a modular, extensible engine in pure C
- Understand graphics programming fundamentals (not just use them)
- Develop clean subsystem architecture (renderer, input, etc.)
- Avoid black-box abstractions
- Maintain full control over memory and data structures

## Build Philosophy
- **No AI-written code** — everything is written and understood manually
- **Learn by building** — not by stitching together libraries
- **Keep it simple first** — complexity comes later
- **Working > Perfect** — every milestone must run

## Tech Stack
- Language: C (C99 or later)
- Windowing/Input: GLFW
- OpenGL Loader: GLAD
- Graphics API: OpenGL (4.x)

## Getting Started
Getting started docs can be found for both Windows and Macbook systems in [init.md](https://github.com/ryanbourdais/Bour-Engine/blob/main/docs/init.md)

## Current Architecture
```
src/
├── main.c          # Entry point
├── controller/     # Input handling
├── external/       # Glad libraries
├── renderer/       # Rendering system (OpenGL)
  ├── shaders/      # GLSL Shader storage
├── utils/          # Shared utilities
```
