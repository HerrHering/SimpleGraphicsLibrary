#include "PingPongBuffer.hpp"
#include "Shader.hpp"
#include "QuadRenderer.hpp"
#include "Logger.hpp"
#include <GLFW/glfw3.h>

PingPongBuffer::~PingPongBuffer() {
    cleanup();
}

void PingPongBuffer::cleanup() {
    if (glfwGetCurrentContext() != nullptr) {
        if (fbos[0] != 0) glDeleteFramebuffers(1, &fbos[0]);
        if (fbos[1] != 0) glDeleteFramebuffers(1, &fbos[1]);
    }
    fbos[0] = 0;
    fbos[1] = 0;
}

void PingPongBuffer::init(int w, int h, GLint internalFormat, GLenum format, GLenum type) {
    cleanup();

    width = w;
    height = h;

    glGenFramebuffers(2, fbos);

    for (int i = 0; i < 2; ++i) {
        textures[i].create(width, height, internalFormat, format, type, nullptr);
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
        // Attach color texture to Framebuffer Attachment 0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i].id, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR << "PingPong Framebuffer [" << i << "] initialization incomplete!\n";
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PingPongBuffer::step(const Shader& stageShader, const QuadRenderer& quad) {
    int writeIndex = 1 - readIndex;

    // 1. Set Viewport to texture resolution
    glViewport(0, 0, width, height);

    // 2. Bind Destination Framebuffer (Write Texture)
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[writeIndex]);

    // 3. Bind Source Texture (Read Texture) to slot 0
    textures[readIndex].bind(0);

    // 4. Run Stage Shader
    stageShader.use();
    stageShader.setInt("u_InputTexture", 0); // Store texture bind slot
    stageShader.setVec2("u_Resolution", glm::vec2(width, height));

    quad.draw();

    // 5. Unbind Framebuffer & Texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    textures[readIndex].unbind();

    // 6. Swap Read / Write indices for next stage iteration
    readIndex = writeIndex;
}