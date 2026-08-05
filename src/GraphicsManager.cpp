#include "GraphicsManager.hpp"
#include "OpenGLDebug.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Defaults {
    constexpr const char* Vert2D = R"(
        #version 460 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aUV;

        out vec2 v_UV;

        uniform mat4 u_Projection;
        uniform mat4 u_Model;

        void main() {
            v_UV = aUV;
            gl_Position = u_Projection * u_Model * vec4(aPos, 0.0, 1.0);
        }
    )";

    constexpr const char* Frag2D = R"(
        #version 460 core
        out vec4 FragColor;
        in vec2 v_UV;

        uniform vec4 u_Color;

        void main() {
            FragColor = u_Color;
        }
    )";
}

GraphicsManager::GraphicsManager(int width, int height)
    : screenWidth(width), screenHeight(height) {
    
    // Bottom-Left origin: Y=0 at Bottom, Y=screenHeight at Top
    orthoProjection = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight), -1.0f, 1.0f);
    fullscreenQuad = std::make_unique<QuadRenderer>();
    primitives2D = std::make_unique<Primitives2D>();

    compileDefaultShaders();
}

void GraphicsManager::updateViewport(int newWidth, int newHeight) {
    screenWidth = newWidth;
    screenHeight = newHeight;
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Maintain Bottom-Left origin on window resize
    orthoProjection = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight), -1.0f, 1.0f);
}

void GraphicsManager::compileDefaultShaders() {
    createShaderFromSource("Default2D", Defaults::Vert2D, Defaults::Frag2D);
}

Shader* GraphicsManager::createShaderFromSource(const std::string& name, const std::string& vert, const std::string& frag, const std::string& geom) {
    auto shader = std::make_unique<Shader>();
    if (shader->compileFromSource(vert, frag, geom)) {
        Shader* rawPtr = shader.get();
        shaderRegistry[name] = std::move(shader);
        return rawPtr;
    }
    std::cerr << "[GraphicsManager] Failed to register shader source: " << name << "\n";
    return nullptr;
}

Shader* GraphicsManager::createShaderFromFiles(const std::string& name, const std::filesystem::path& vertPath, const std::filesystem::path& fragPath, const std::filesystem::path& geomPath) {
    auto shader = std::make_unique<Shader>();
    if (shader->compileFromFiles(vertPath, fragPath, geomPath)) {
        Shader* rawPtr = shader.get();
        shaderRegistry[name] = std::move(shader);
        return rawPtr;
    }
    std::cerr << "[GraphicsManager] Failed to register shader from file paths: " << name << "\n";
    return nullptr;
}

Shader* GraphicsManager::getShader(const std::string& name) {
    if (auto it = shaderRegistry.find(name); it != shaderRegistry.end()) {
        return it->second.get();
    }
    std::cerr << "[GraphicsManager Warning] Shader requested is non-existent: " << name << "\n";
    return nullptr;
}

void GraphicsManager::drawFullscreenQuad(const std::string& shaderName) {
    Shader* shader = getShader(shaderName);
    if (shader) {
        shader->use();
        fullscreenQuad->draw();
    }
}

void GraphicsManager::drawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float thickness) {
    Shader* default2D = getShader("Default2D");
    if (default2D) {
        primitives2D->drawLine(start, end, color, thickness, *default2D, orthoProjection);
    }
}

void GraphicsManager::drawRectangle(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
    Shader* default2D = getShader("Default2D");
    if (default2D) {
        primitives2D->drawRectangle(pos, size, color, *default2D, orthoProjection);
    }
}

void GraphicsManager::drawCircle(const glm::vec2& center, float radius, const glm::vec4& color) {
    Shader* default2D = getShader("Default2D");
    if (default2D) {
        primitives2D->drawCircle(center, radius, color, *default2D, orthoProjection);
    }
}

void GraphicsManager::unbindAll() {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}