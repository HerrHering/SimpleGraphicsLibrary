# Modern C++20 OpenGL Graphics Library

A lightweight, boilerplate-free C++20 graphics framework built on **OpenGL 4.6 Core**, **GLFW**, **GLM**, and **GLAD**. Designed for rapid prototyping of raymarching fragment shaders, post-processing effects, and Raylib-style 2D HUD/primitive drawing with zero manual resource management.

---

## 📑 Table of Contents

0. [🚀 Tagline](#-tagline)
1. [🛠️ GLAD Configuration & Setup](#️-glad-configuration--setup)
2. [💻 System Requirements](#-system-requirements)
3. [🚀 Quick Start (Building)](#-quick-start-building)
4. [✅ What CAN Be Done](#-what-can-be-done)
5. [❌ What CANNOT Be Done (Out of Scope)](#-what-cannot-be-done-out-of-scope)
6. [📁 Source Code Map](#-source-code-map)
7. [🔍 Demo Application Architecture](#-demo-application-architecture)
8. [🎮 Testing & Interactive Controls](#-testing--interactive-controls)

---

## 🚀 Tagline

Forget boilerplate. `#include` a header, call a constructor, and start drawing — no engine, no build pipeline, no ceremony.

---

## 🛠️ GLAD Configuration & Setup

This repository includes pre-generated GLAD files in [`thirdparty/glad/`](thirdparty/glad/). If you encounter GLAD header mismatches or need to regenerate the loader files for your platform, follow these steps:

1. Visit the online GLAD generator: **[https://gen.glad.sh/](https://gen.glad.sh/)**
2. Configure the options as follows:
   - **Language**: `C/C++`
   - **Specification**: `OpenGL`
   - **API Version**: `Version 4.6` (or minimum `4.3`)
   - **Profile**: `Core`
   - **Options**: Select `Loader`
3. Click **Generate** and download the resulting ZIP archive.
4. Extract the ZIP contents into the `thirdparty/glad` directory so that it matches this layout:
   ```text
   thirdparty/glad/
   ├── include/
   │   ├── glad/
   │   │   └── gl.h
   │   └── KHR/
   │       └── khrplatform.h
   └── src/
       └── gl.c
   ```

---

## 💻 System Requirements

- **C++ Standard**: C++20 compatible compiler (GCC 11+, Clang 13+, or MSVC 2019 16.10+)
- **Build System**: CMake 3.20 or newer
- **OpenGL Context**: GPU driver with OpenGL 4.6 (or minimum 4.3) Core Profile support
- **Git**: Required by CMake `FetchContent` to download GLFW (v3.4) and GLM (v1.0.1) automatically

---

## 🚀 Quick Start (Building)

```bash
# 1. Clone repository
git clone <repository-url>
cd SimpleGraphicsLibrary

# 2. Configure project with CMake
cmake -B build

# 3. Build the library and demo executables
cmake --build build

# 4. Run a demo
./build/bin/GraphicsManagerApp_Raymarching_Demo   # FPS camera + hot-swappable raymarched SDF primitives
./build/bin/GraphicsManagerApp_Textures_Demo      # Async PBO texture upload + GPU Game of Life
```

> These two demo executables are only built when this repo is configured as the top-level CMake project (i.e. not when consumed as a subproject/dependency).

---

## ✅ What CAN Be Done

### 1. Unified Bottom-Left Origin Coordinate System
All elements share the native OpenGL Cartesian coordinate space ($+X$ Right, $+Y$ Up, $(0,0)$ at Bottom-Left):
- **2D Pixel Space**: Position `(100, 50)` places a shape 100px from the left and 50px from the **bottom** edge of the screen.
- **UV Space**: Quads output UVs ranging from `(0,0)` at Bottom-Left to `(1,1)` at Top-Right.

### 2. Centralized State & Resource Management (RAII)
- [`GraphicsManager`](include/GraphicsManager.hpp) ([`src/GraphicsManager.cpp`](src/GraphicsManager.cpp)) manages shader compilation, primitive renderers, and full-screen quad passes without manual `glDelete*` management.
- **Segfault-Proof Context Guards**: [`Shader`](src/Shader.cpp), [`QuadRenderer`](include/QuadRenderer.hpp), and [`Primitives2D`](src/Primitives2D.cpp) check `glfwGetCurrentContext()` during destruction, preventing driver crashes if the window closes before objects leave scope.

### 3. C++20 Source-Location Logger & Stream Hijacker
- Intercepts `std::cerr` line-by-line via [`Logger.hpp`](include/Logger.hpp).
- Uses C++20 `<source_location>` macros (`LOG_ERROR`, `LOG_WARN`, `LOG_INFO`, `LOG_DEBUG`) to output exact source filenames, line numbers, and function names.
- Auto-enables ANSI color coding (Red = Error, Yellow = Warning, Cyan = Info) and severity threshold filtering.

### 4. Fullscreen Pixel / Raymarching Pass
- [`QuadRenderer`](include/QuadRenderer.hpp) provides a full-screen NDC quad.
- Attributes are standardized: `location = 0` (`aPos`), `location = 1` (`aUV`).
- [`SDFScene`](include/SDFScene.hpp) demonstrates raymarching 3D primitives (Sphere, Box, Torus) with dynamic macro swapping (`ShaderLoader::replaceMacro` in [`include/ShaderLoader.hpp`](include/ShaderLoader.hpp)).

### 5. 2D Primitive Drawing (Raylib Style)
- [`Primitives2D`](include/Primitives2D.hpp) ([`src/Primitives2D.cpp`](src/Primitives2D.cpp)) provides screen-space pixel rendering for:
  - Lines (`drawLine`)
  - Rectangles (`drawRectangle`)
  - Circles (`drawCircle`)

### 6. Verbose Shader Debugging & Driver Diagnostics
- [`Shader`](include/Shader.hpp) ([`src/Shader.cpp`](src/Shader.cpp)) prints line-numbered GLSL source code dumps on compilation failures and warns on missing/optimized-out uniforms.
- [`OpenGLDebug.hpp`](include/OpenGLDebug.hpp) registers synchronous OpenGL driver message callbacks.

### 7. 3D FPS Navigation
- [`Camera`](include/Camera.hpp) provides an Euler-angle 3D camera with view-matrix calculation.

### 8. Texture Upload (Sync & Async)
- [`Texture`](include/Texture.hpp) ([`src/Texture.cpp`](src/Texture.cpp)) is an RAII 2D texture wrapper supporting both blocking (`updatePixels`, `glTexSubImage2D`) and non-blocking DMA uploads (`updatePixelsAsync`, via a `GL_PIXEL_UNPACK_BUFFER` PBO) so CPU-generated pixel data can be streamed to the GPU without stalling the render thread.
- [`GraphicsManager::drawTexture`](include/GraphicsManager.hpp) / `drawTextureFullscreen` render a `Texture` in 2D screen space.

### 9. GPU Iterative Compute (Ping-Pong Buffers)
- [`PingPongBuffer`](include/PingPongBuffer.hpp) ([`src/PingPongBuffer.cpp`](src/PingPongBuffer.cpp)) owns 2 FBOs + 2 textures and alternates read/write between them each `step()`, enabling multi-pass GPU simulations (e.g. cellular automata, blur chains) driven entirely by a fragment shader.

### 10. Image Export
- [`ImageSaver`](include/ImageSaver.hpp) ([`src/ImageSaver.cpp`](src/ImageSaver.cpp)) writes CPU pixel buffers or GPU `Texture` contents to timestamped PNG files (via `stb_image_write`), handling channel counts and the bottom-left → top-left vertical flip automatically.

---

## ❌ What CANNOT Be Done (Out of Scope)

To maintain a minimal footprint, the following features are **not supported** out of the box:

- **No 3D Model Loading**: Cannot load mesh files (`.obj`, `.fbx`, `.gltf`). Geometry is limited to 2D primitives and procedural screen-quad fragment shaders.
- **No Image-File Texture Loading**: `Texture` uploads are fed from CPU-generated/computed pixel buffers, not decoded image files — there's no `stb_image`-based loader for `.png`/`.jpg` into a `Texture` (stb is only used by [`ImageSaver`](include/ImageSaver.hpp) for *writing* PNGs out, not reading them in).
- **No Automatic 2D Batching**: Each 2D primitive shape issues an individual draw call (`glDrawArrays`). Large-scale 2D particle/sprite batching is not implemented.
- **No Text / Font Rasterization**: No TTF/Bitmap font rendering engine.
- **No General-Purpose Multi-Pass FBO Manager**: [`PingPongBuffer`](include/PingPongBuffer.hpp) provides a fixed 2-FBO ping-pong pair for iterative GPU compute, but there's no arbitrary N-pass framebuffer chain/graph — most rendering still targets the default window swapchain buffer directly.
- **No Physics or Audio**: Focuses exclusively on graphics rendering.

---

## 📁 Source Code Map

| Feature / Responsibility | Core Files |
| :--- | :--- |
| **Raymarching Demo App** | [`src/RaymarchingDemo.cpp`](src/RaymarchingDemo.cpp) |
| **Textures Demo App** | [`src/TexturesDemo.cpp`](src/TexturesDemo.cpp) |
| **Central Manager** | [`include/GraphicsManager.hpp`](include/GraphicsManager.hpp) / [`src/GraphicsManager.cpp`](src/GraphicsManager.cpp) |
| **Shader Compilation & Uniform Caching** | [`include/Shader.hpp`](include/Shader.hpp) / [`src/Shader.cpp`](src/Shader.cpp) |
| **Source File Reading & Macro Swapping** | [`include/ShaderLoader.hpp`](include/ShaderLoader.hpp) |
| **2D Shape Renderer** | [`include/Primitives2D.hpp`](include/Primitives2D.hpp) / [`src/Primitives2D.cpp`](src/Primitives2D.cpp) |
| **Fullscreen Screen Quad** | [`include/QuadRenderer.hpp`](include/QuadRenderer.hpp) |
| **Raymarching SDF Scene** | [`include/SDFScene.hpp`](include/SDFScene.hpp) |
| **C++20 Logging & `std::cerr` Hijack** | [`include/Logger.hpp`](include/Logger.hpp) |
| **OpenGL Driver Debug Context** | [`include/OpenGLDebug.hpp`](include/OpenGLDebug.hpp) |
| **3D Camera System** | [`include/Camera.hpp`](include/Camera.hpp) |
| **2D Texture (Sync & Async PBO Upload)** | [`include/Texture.hpp`](include/Texture.hpp) / [`src/Texture.cpp`](src/Texture.cpp) |
| **Ping-Pong GPU Compute Buffer** | [`include/PingPongBuffer.hpp`](include/PingPongBuffer.hpp) / [`src/PingPongBuffer.cpp`](src/PingPongBuffer.cpp) |
| **PNG Image Export** | [`include/ImageSaver.hpp`](include/ImageSaver.hpp) / [`src/ImageSaver.cpp`](src/ImageSaver.cpp) |

---

## 🔍 Demo Application Architecture

The two demo executables both do their own GLFW/GLAD/`Logger`/`OpenGLDebug` setup and construct their own `GraphicsManager` — there is no shared `main.cpp`.

### `RaymarchingDemo.cpp` — FPS camera + raymarched SDF primitives

1. **Init**: `Logger logger(LogSeverity::Info)` hijacks `std::cerr`; GLFW window created with an OpenGL 4.6 Core + Debug Context; `gladLoadGL` loads driver function pointers; `setupOpenGLDebugging()` registers the GL debug callback; `GraphicsManager gfx(ctx->width, ctx->height)` pre-compiles internal default 2D shaders and sets up the bottom-left pixel orthographic projection.
2. **Render loop** (`while (!glfwWindowShouldClose(window))`):
   - **Camera update**: `processInput()` + the mouse callback drive an FPS-style [`Camera`](include/Camera.hpp) using frame `deltaTime`.
   - **Hot-reloadable shader compile**: if `ctx->pendingRecompile` is set (on startup, or when key `1`/`2`/`3` swaps `ctx->scene.currentPrimitive`), recompiles the raymarching pipeline via `gfx.createShaderFromSource("Raymarch", SDFScene::getDefaultVertexShaderSource(), ctx->scene.getFragmentShaderSource())`, using [`SDFScene`](include/SDFScene.hpp)/[`ShaderLoader`](include/ShaderLoader.hpp) macro swapping.
   - **Pass 1 — raymarching**: sends `u_Resolution`, `u_InvView` (inverse of `camera.getViewMatrix()`), `u_Time` uniforms, then `gfx.drawFullscreenQuad("Raymarch")` via [`QuadRenderer`](include/QuadRenderer.hpp).
   - **Pass 2 — 2D HUD overlay**: `gfx.drawRectangle()` / `drawCircle()` / `drawLine()` from [`Primitives2D`](include/Primitives2D.hpp) draw a status box, LED indicator, and center crosshair over the 3D scene.
   - **State reset**: `gfx.unbindAll()` clears VAO/VBO/Shader bindings for the next frame.
3. **Cleanup**: `glfwDestroyWindow` + `glfwTerminate`; all wrappers safely no-op via `glfwGetCurrentContext()` guards in their destructors.

### `TexturesDemo.cpp` — async texture upload + GPU Game of Life

1. **Init**: same Logger/GLFW/GLAD/`setupOpenGLDebugging()`/`GraphicsManager` setup as above (no camera, no key callbacks).
2. **Workflow 1 — CPU-generated texture, async PBO upload**: each frame, a `Texture` is filled with an animated plasma pattern in a CPU pixel buffer, then pushed to the GPU non-blockingly via `Texture::updatePixelsAsync` (PBO DMA transfer).
3. **Workflow 2 — GPU Game of Life via ping-pong compute**: a `PingPongBuffer` is seeded with `std::mt19937`-randomized noise, then stepped twice per frame (`pingPong.step(*simShader, dummyQuad)`) running a Game-of-Life fragment shader (`Shaders::GameOfLifeFrag`) that reads neighbor cells from the input texture and writes the next generation to the output texture.
4. **Render**: both results are drawn side by side on screen via `gfx.drawTexture(...)`, then `gfx.unbindAll()`.

---

## 🎮 Testing & Interactive Controls

### Raymarching Demo (`./build/bin/GraphicsManagerApp_Raymarching_Demo`)

**3D FPS Camera Controls**
- **`W` / `A` / `S` / `D`**: Move Camera Forward / Left / Backward / Right
- **`Space`**: Fly Camera Upward (+Y)
- **`Left Ctrl`**: Fly Camera Downward (-Y)
- **`Mouse Movement`**: Look / Rotate Camera View in 3D Space

**Real-Time Shader Pipeline Hot-Swapping**
Press numeric keys to dynamically replace the SDF macro call in the active fragment shader and trigger on-the-fly GLSL recompilation:
- **`1`**: Switch 3D Raymarched Primitive to **Sphere** (`sdSphere`)
- **`2`**: Switch 3D Raymarched Primitive to **Box** (`sdBox`)
- **`3`**: Switch 3D Raymarched Primitive to **Torus** (`sdTorus`)

**Application Controls**
- **`Escape`**: Close application and trigger clean destruction

### Textures Demo (`./build/bin/GraphicsManagerApp_Textures_Demo`)

This demo is non-interactive — it registers no key callback, so `Escape` does nothing here. Close the window (OS close button) to exit and trigger clean destruction.