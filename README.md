markdown
# Modern C++20 OpenGL Graphics Library

A lightweight, boilerplate-free C++20 graphics framework built on **OpenGL 4.6 Core**, **GLFW**, **GLM**, and **GLAD**. Designed for rapid prototyping of raymarching fragment shaders, post-processing effects, and Raylib-style 2D HUD/primitive drawing with zero manual resource management.

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