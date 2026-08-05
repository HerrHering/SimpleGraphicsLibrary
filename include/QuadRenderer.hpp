/**
 * @file QuadRenderer.hpp
 * @brief Fullscreen NDC Quad Mesh geometry encapsulation for Raymarching / Screen Effects.
 *
 * @example Usage Example:
 * @code
 * #include "QuadRenderer.hpp"
 * #include "Shader.hpp"
 *
 * void renderRaymarchPass(const Shader& raymarchShader) {
 *     static QuadRenderer screenQuad;
 *     
 *     raymarchShader.use();
 *     // Draw full-screen quad through the active shader program
 *     screenQuad.draw();
 * }
 * @endcode
 */


#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h> // Included for glfwGetCurrentContext()

/**
 * @class QuadRenderer
 * @brief Fullscreen NDC [-1.0, 1.0] Quad with Bottom-Left UV origin.
 * 
 * Vertex Attributes:
 *   location 0: vec2 aPos
 *   location 1: vec2 aUV (Bottom-Left = 0,0 | Top-Right = 1,1)
 */
class QuadRenderer {
public:
    GLuint vao = 0;
    GLuint vbo = 0;

    QuadRenderer() {
        // Interleaved: Position (NDC vec2) | UV (vec2, Bottom-Left Origin)
        constexpr float quadVertices[] = {
            // Position     // UV (0,0 is Bottom-Left)
            -1.0f, -1.0f,   0.0f, 0.0f, // Bottom-Left
             1.0f, -1.0f,   1.0f, 0.0f, // Bottom-Right
             1.0f,  1.0f,   1.0f, 1.0f, // Top-Right

            -1.0f, -1.0f,   0.0f, 0.0f, // Bottom-Left
             1.0f,  1.0f,   1.0f, 1.0f, // Top-Right
            -1.0f,  1.0f,   0.0f, 1.0f  // Top-Left
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        const GLsizei stride = 4 * sizeof(float);

        // Location 0: Position (vec2)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));

        // Location 1: UV Coordinates (vec2)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(2 * sizeof(float)));

        glBindVertexArray(0);
    }

    ~QuadRenderer() {
        if (glfwGetCurrentContext() != nullptr) { // If context was deleted, these glDelete functions will cause SegFaults
            if (vbo != 0) glDeleteBuffers(1, &vbo);
            if (vao != 0) glDeleteVertexArrays(1, &vao);
        }
    }

    QuadRenderer(const QuadRenderer&) = delete;
    QuadRenderer& operator=(const QuadRenderer&) = delete;

    void draw() const {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
};