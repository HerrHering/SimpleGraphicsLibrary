/**
 * @file Texture.hpp
 * @brief RAII OpenGL 2D Texture encapsulation with PBO (Pixel Buffer Object) async uploads to prevent main thread blocking.
 *
 * @details
 * OPENGL GLSL ASSUMPTIONS:
 * - Texture coordinates follow Bottom-Left origin convention: (0,0) = Bottom-Left, (1,1) = Top-Right.
 * - Supports both standard dynamic uploads (glTexSubImage2D) and non-blocking asynchronous DMA uploads (GL_PIXEL_UNPACK_BUFFER PBOs).
 *
 * @example Usage Example:
 * @code
 * #include "Texture.hpp"
 * 
 * Texture tex;
 * tex.create(512, 512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
 * 
 * // Asynchronously upload raw CPU pixels via PBO without blocking CPU
 * std::vector<uint8_t> pixels = generateCpuPixels();
 * tex.updatePixelsAsync(pixels.data(), pixels.size());
 * 
 * // Bind to texture unit 0
 * tex.bind(0);
 * @endcode
 */

#pragma once
#include <glad/gl.h>
#include <cstddef>

class Texture {
public:
    GLuint id = 0;
    GLuint pboID = 0;
    int width = 0;
    int height = 0;
    GLint internalFormat = GL_RGBA8;
    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;

    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    /**
    * @brief Allocates GPU VRAM storage for a 2D texture and optionally uploads initial pixel data.
    *
    * @param w Width of the texture in pixels (must be > 0).
    * @param h Height of the texture in pixels (must be > 0).
    * @param intFormat OpenGL internal storage format on the GPU VRAM 
    *                  (e.g., GL_RGBA8, GL_RGB8, GL_RGBA16F, GL_R8). Defaults to GL_RGBA8.
    * @param srcFormat Format of the incoming CPU pixel data channels 
    *                  (e.g., GL_RGBA, GL_RGB, GL_RED). Defaults to GL_RGBA.
    * @param dataFormat Data type of each pixel component in CPU memory 
    *                   (e.g., GL_UNSIGNED_BYTE, GL_FLOAT). Defaults to GL_UNSIGNED_BYTE.
    * @param initialPixels Optional pointer to CPU memory containing initial pixel bytes. 
    *                      Pass nullptr to allocate uninitialized VRAM storage.
    */
    void create(int w, int h, GLint intFormat = GL_RGBA8, GLenum srcFormat = GL_RGBA, GLenum dataFormat = GL_UNSIGNED_BYTE, const void* initialPixels = nullptr);
    
    // Standard synchronous pixel update
    void updatePixels(const void* pixelData);

    // Non-blocking Asynchronous DMA upload via Pixel Buffer Object (PBO)
    void updatePixelsAsync(const void* pixelData, size_t dataSizeInBytes);

    void bind(unsigned int slot = 0) const;
    void unbind() const;

private:
    void cleanup();
    void initPBO(size_t dataSizeInBytes);
};