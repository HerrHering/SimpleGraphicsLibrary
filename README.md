# Modern C++20 OpenGL Graphics Library

A lightweight, boilerplate-free C++20 graphics framework built on **OpenGL 4.6 Core**, **GLFW**, **GLM**, and **GLAD**. Designed for rapid prototyping of raymarching fragment shaders, post-processing effects, and Raylib-style 2D HUD/primitive drawing with zero manual resource management.

---

## 📑 Table of Contents

1. [🛠️ GLAD Configuration & Setup](#️-glad-configuration--setup)
2. [💻 System Requirements](#-system-requirements)
3. [🚀 Quick Start (Building)](#-quick-start-building)
4. [✅ What CAN Be Done](#-what-can-be-done)
5. [❌ What CANNOT Be Done (Out of Scope)](#-what-cannot-be-done-out-of-scope)
6. [📁 Source Code Map](#-source-code-map)
7. [🔍 Main Application Architecture (`main.cpp` Step-by-Step)](#-main-application-architecture-maincpp-step-by-step)
8. [🎮 Testing & Interactive Controls](#-testing--interactive-controls)

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

# 3. Build executable
cmake --build build

# 4. Run executable
./build/bin/GraphicsManagerApp
```

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

---

## ❌ What CANNOT Be Done (Out of Scope)

To maintain a minimal footprint, the following features are **not supported** out of the box:

- **No 3D Model Loading**: Cannot load mesh files (`.obj`, `.fbx`, `.gltf`). Geometry is limited to 2D primitives and procedural screen-quad fragment shaders.
- **No Built-in Texture / Sprite Loader**: Texture loading libraries (e.g. `stb_image`) are not bundled. 2D rendering is currently limited to solid RGBA colors.
- **No Automatic 2D Batching**: Each 2D primitive shape issues an individual draw call (`glDrawArrays`). Large-scale 2D particle/sprite batching is not implemented.
- **No Text / Font Rasterization**: No TTF/Bitmap font rendering engine.
- **No Framebuffer Object (FBO) Chain**: Rendering is directed straight to the default window swapchain buffer (no built-in multi-pass framebuffer manager).
- **No Physics or Audio**: Focuses exclusively on graphics rendering.

---

## 📁 Source Code Map

| Feature / Responsibility | Core Files |
| :--- | :--- |
| **Main Application Loop** | [`src/main.cpp`](src/main.cpp) |
| **Central Manager** | [`include/GraphicsManager.hpp`](include/GraphicsManager.hpp) / [`src/GraphicsManager.cpp`](src/GraphicsManager.cpp) |
| **Shader Compilation & Uniform Caching** | [`include/Shader.hpp`](include/Shader.hpp) / [`src/Shader.cpp`](src/Shader.cpp) |
| **Source File Reading & Macro Swapping** | [`include/ShaderLoader.hpp`](include/ShaderLoader.hpp) |
| **2D Shape Renderer** | [`include/Primitives2D.hpp`](include/Primitives2D.hpp) / [`src/Primitives2D.cpp`](src/Primitives2D.cpp) |
| **Fullscreen Screen Quad** | [`include/QuadRenderer.hpp`](include/QuadRenderer.hpp) |
| **Raymarching SDF Scene** | [`include/SDFScene.hpp`](include/SDFScene.hpp) |
| **C++20 Logging & `std::cerr` Hijack** | [`include/Logger.hpp`](include/Logger.hpp) |
| **OpenGL Driver Debug Context** | [`include/OpenGLDebug.hpp`](include/OpenGLDebug.hpp) |
| **3D Camera System** | [`include/Camera.hpp`](include/Camera.hpp) |

---

## 🔍 Main Application Architecture (`main.cpp` Step-by-Step)

The sample application in [`src/main.cpp`](src/main.cpp) ties all sub-systems together into a multi-pass pipeline:

### Step 1: Logging & Window Context Initialization
```cpp
Logger logger(LogSeverity::Info);
```
- Instantiates [`Logger`](include/Logger.hpp) to hijack `std::cerr`, set up ANSI terminal colors, and capture source location metadata (`[main.cpp:56 (main)] [Info]`).
- Configures GLFW hints for an OpenGL 4.6 Core Profile with a Debug Context enabled.

### Step 2: Driver Function Loading & Debug Registration
```cpp
gladLoadGL(glfwGetProcAddress);
setupOpenGLDebugging();
```
- Loads function pointers via GLAD.
- Calls `setupOpenGLDebugging()` from [`OpenGLDebug.hpp`](include/OpenGLDebug.hpp) to register driver callback warnings and errors.

### Step 3: Graphics Manager Setup
```cpp
GraphicsManager gfx(ctx->width, ctx->height);
```
- Instantiates [`GraphicsManager`](include/GraphicsManager.hpp), which pre-compiles internal default 2D shaders, sets up the bottom-left pixel orthographic projection matrix, and allocates geometry renderers.

### Step 4: Render Loop & Dynamic Pipeline Recompilation
Inside the main `while (!glfwWindowShouldClose(window))` loop:

1. **Camera & Movement Update**: Calls `processInput()` and `mouseCallback()` to update the [`Camera`](include/Camera.hpp) view parameters using frame `deltaTime`.
2. **Hot-Reloadable Shader Compile**: Checks `ctx->pendingRecompile`. If `true` (on startup or when key `1`, `2`, or `3` is pressed):
   ```cpp
   gfx.createShaderFromSource("Raymarch", SDFScene::getDefaultVertexShaderSource(), ctx->scene.getFragmentShaderSource());
   ```
   Uses [`SDFScene`](include/SDFScene.hpp) and [`ShaderLoader`](include/ShaderLoader.hpp) to dynamically swap GLSL macro definitions and recompile the raymarching pipeline via [`Shader`](include/Shader.hpp).
3. **Pass 1 — 3D Raymarching Pass**:
   - Computes the `invView` matrix from `ctx->camera.getViewMatrix()`.
   - Sends uniforms (`u_Resolution`, `u_InvView`, `u_Time`) to the active raymarching shader.
   - Calls `gfx.drawFullscreenQuad("Raymarch")` to execute a full-screen fragment raymarching pass via [`QuadRenderer`](include/QuadRenderer.hpp).
4. **Pass 2 — 2D HUD Overlay Pass**:
   - Calls `gfx.drawRectangle()`, `gfx.drawCircle()`, and `gfx.drawLine()` from [`Primitives2D`](include/Primitives2D.hpp) to render screen-space UI elements (a status bar, LED indicator, and center crosshair) over the 3D raymarched scene.
5. **State Reset**:
   - Calls `gfx.unbindAll()` to reset active VAO, VBO, and Shader bindings, keeping state clean for the next frame.

### Step 5: Graceful Cleanup
```cpp
glfwDestroyWindow(window);
glfwTerminate();
```
- Destroys the window and window context cleanly. All OpenGL wrappers safely handle this termination due to `glfwGetCurrentContext()` safety guards inside their destructors.

---

## 🎮 Testing & Interactive Controls

Launch the compiled executable (`./build/bin/GraphicsManagerApp`) to test the framework in real-time:

### 3D FPS Camera Controls
- **`W` / `A` / `S` / `D`**: Move Camera Forward / Left / Backward / Right
- **`Space`**: Fly Camera Upward (+Y)
- **`Left Ctrl`**: Fly Camera Downward (-Y)
- **`Mouse Movement`**: Look / Rotate Camera View in 3D Space

### Real-Time Shader Pipeline Hot-Swapping
Press numeric keys to dynamically replace the SDF macro call in the active fragment shader and trigger on-the-fly GLSL recompilation:
- **`1`**: Switch 3D Raymarched Primitive to **Sphere** (`sdSphere`)
- **`2`**: Switch 3D Raymarched Primitive to **Box** (`sdBox`)
- **`3`**: Switch 3D Raymarched Primitive to **Torus** (`sdTorus`)

### Application Controls
- **`Escape`**: Close application and trigger clean destruction