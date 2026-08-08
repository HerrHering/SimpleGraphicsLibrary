/**
 * @file PingPongBuffer.hpp
 * @brief Dual Framebuffer (FBO) render-to-texture ping-pong architecture for iterative GPU computation.
 *
 * @details
 * OPENGL DETAILS:
 * - Creates 2 FBOs and 2 attached Textures (Texture A and Texture B).
 * - In stage N: Shader reads from Texture A, renders into FBO B (Texture B).
 * - Then pointers swap (Ping-Pong), making Texture B the input for stage N+1.
 *
 * SHADER ASSUMPTIONS:
 *  in vec2 v_UV;
 *  uniform sampler2D u_InputTexture;
 *  uniform vec2 u_Resolution;
 *
 * @example Usage Example:
 * @code
 * PingPongBuffer pingPong;
 * pingPong.init(512, 512);
 * 
 * // Perform 10 GPU processing passes in parallel
 * for (int i = 0; i < 10; ++i) {
 *     pingPong.step(iterationShader, quadRenderer);
 * }
 * 
 * // Retrieve result texture to draw to screen
 * const Texture& result = pingPong.getReadTexture();
 * @endcode
 */

#pragma once
#include "Texture.hpp"

class Shader;
class QuadRenderer;

class PingPongBuffer {
public:
    PingPongBuffer() = default;
    ~PingPongBuffer();

    PingPongBuffer(const PingPongBuffer&) = delete;
    PingPongBuffer& operator=(const PingPongBuffer&) = delete;

    void init(int width, int height, GLint internalFormat = GL_RGBA8, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);
    
    // Executes 1 GPU iterative computation pass
    void step(const Shader& stageShader, const QuadRenderer& quad);

    [[nodiscard]] const Texture& getReadTexture() const { return textures[readIndex]; }
    [[nodiscard]] const Texture& getWriteTexture() const { return textures[1 - readIndex]; }

private:
    GLuint fbos[2] = {0, 0};
    Texture textures[2];
    int readIndex = 0;
    int width = 0;
    int height = 0;

    void cleanup();
};