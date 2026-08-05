/**
 * @file Primitives2D.hpp
 * @brief Immediate/Simple batch drawing for 2D shape primitives (Lines, Rectangles, Circles).
 *
 * COORDINATE SYSTEM:
 * - Uses Top-Left origin pixel screen space when paired with an orthographic projection matrix:
 *     glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);
 *
 * @example Usage Example:
 * @code
 * #include "Primitives2D.hpp"
 * #include "Shader.hpp"
 * #include <glm/gtc/matrix_transform.hpp>
 *
 * void drawUI(Primitives2D& renderer, const Shader& shader2D) {
 *     glm::mat4 ortho = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f);
 *
 *     // Draw rectangle at pos (x=50, y=50) of size 100x200 with RGBA color
 *     renderer.drawRectangle(glm::vec2(50, 50), glm::vec2(100, 200), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), shader2D, ortho);
 *
 *     // Draw circle centered at (x=640, y=360) with radius 25
 *     renderer.drawCircle(glm::vec2(640, 360), 25.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), shader2D, ortho);
 * }
 * @endcode
 */

#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

/**
 * @class Primitives2D
 * @brief Raylib-style 2D primitive shape renderer (Line, Circle, Square, Rectangle).
 * 
 *     layout (location = 0) in vec2 aPos;
 *     layout (location = 1) in vec2 aTexCoord;
 *     uniform mat4 u_Projection;
 *     uniform mat4 u_Model;
 *     uniform vec4 u_Color;
 */
class Primitives2D {
public:
    Primitives2D();
    ~Primitives2D();

    Primitives2D(const Primitives2D&) = delete;
    Primitives2D& operator=(const Primitives2D&) = delete;

    void drawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float thickness, const Shader& activeShader, const glm::mat4& projection);
    void drawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const Shader& activeShader, const glm::mat4& projection);
    void drawCircle(const glm::vec2& center, float radius, const glm::vec4& color, const Shader& activeShader, const glm::mat4& projection);

private:
    GLuint quadVAO = 0, quadVBO = 0;
    GLuint lineVAO = 0, lineVBO = 0;
    GLuint circleVAO = 0, circleVBO = 0;
    size_t circleVertexCount = 0;

    void initQuadMesh();
    void initLineMesh();
    void initCircleMesh(size_t segments = 64);
};