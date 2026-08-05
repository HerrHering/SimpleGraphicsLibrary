#include "Primitives2D.hpp"
#include "Shader.hpp"
#include <GLFW/glfw3.h> // Included for glfwGetCurrentContext()
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <numbers>

Primitives2D::Primitives2D() {
    initQuadMesh();
    initLineMesh();
    initCircleMesh(64);
}

Primitives2D::~Primitives2D() {
    if (glfwGetCurrentContext() != nullptr) { // If context was deleted, these glDelete functions will cause SegFaults
        if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
        if (quadVBO) glDeleteBuffers(1, &quadVBO);
        if (lineVAO) glDeleteVertexArrays(1, &lineVAO);
        if (lineVBO) glDeleteBuffers(1, &lineVBO);
        if (circleVAO) glDeleteVertexArrays(1, &circleVAO);
        if (circleVBO) glDeleteBuffers(1, &circleVBO);
    }
}

void Primitives2D::initQuadMesh() {
    // Interleaved Position (0..1) and UV (0..1) with Bottom-Left Origin
    constexpr float unitQuad[] = {
        // Position   // UV
        0.0f, 0.0f,   0.0f, 0.0f, // Bottom-Left
        1.0f, 0.0f,   1.0f, 0.0f, // Bottom-Right
        1.0f, 1.0f,   1.0f, 1.0f, // Top-Right

        0.0f, 0.0f,   0.0f, 0.0f, // Bottom-Left
        1.0f, 1.0f,   1.0f, 1.0f, // Top-Right
        0.0f, 1.0f,   0.0f, 1.0f  // Top-Left
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unitQuad), unitQuad, GL_STATIC_DRAW);

    const GLsizei stride = 4 * sizeof(float);
    
    // Location 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    
    // Location 1: UV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(2 * sizeof(float)));

    glBindVertexArray(0);
}

void Primitives2D::initLineMesh() {
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Primitives2D::initCircleMesh(size_t segments) {
    std::vector<float> circleVertices;
    circleVertices.push_back(0.0f); // Center X
    circleVertices.push_back(0.0f); // Center Y

    for (size_t i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(i) * 2.0f * std::numbers::pi_v<float> / static_cast<float>(segments);
        circleVertices.push_back(std::cos(angle));
        circleVertices.push_back(std::sin(angle));
    }
    circleVertexCount = circleVertices.size() / 2;

    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Primitives2D::drawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float thickness, const Shader& activeShader, const glm::mat4& projection) {
    activeShader.use();
    activeShader.setMat4("u_Projection", projection);
    activeShader.setMat4("u_Model", glm::mat4(1.0f));
    activeShader.setVec4("u_Color", color);

    float vertices[4] = { start.x, start.y, end.x, end.y };
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glLineWidth(thickness);
    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
    glLineWidth(1.0f);
}

void Primitives2D::drawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const Shader& activeShader, const glm::mat4& projection) {
    activeShader.use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    activeShader.setMat4("u_Projection", projection);
    activeShader.setMat4("u_Model", model);
    activeShader.setVec4("u_Color", color);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Primitives2D::drawCircle(const glm::vec2& center, float radius, const glm::vec4& color, const Shader& activeShader, const glm::mat4& projection) {
    activeShader.use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(radius, radius, 1.0f));

    activeShader.setMat4("u_Projection", projection);
    activeShader.setMat4("u_Model", model);
    activeShader.setVec4("u_Color", color);

    glBindVertexArray(circleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(circleVertexCount));
    glBindVertexArray(0);
}