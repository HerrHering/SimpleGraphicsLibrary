/**
 * @file GraphicsManager.hpp
 * @brief High-level central orchestrator for shaders, state management, and shape rendering.
 *
 * @details
 * RESPONSIBILITIES:
 * - Owns shader lifetime via internal shader registry map.
 * - Automatically registers and builds standard built-in shaders (e.g., "Default2D").
 * - Encapsulates 2D primitive rendering in top-left pixel screen space coordinates.
 * - Provides full-screen quad post-process / raymarching pass helpers.
 * - Prevents state leaks by exposing unbindAll().
 *
 * @example Usage Example:
 * @code
 * #include "GraphicsManager.hpp"
 *
 * int main() {
 *     GraphicsManager gfx(1280, 720); // Initializes screen dimensions
 *
 *     // 1. Create custom raymarching shader
 *     gfx.createShaderFromFiles("Raymarch", "shaders/raymarch.vert", "shaders/raymarch.frag");

 *     while (running) {
 *         gfx.updateViewport(1280, 720);
 *
 *         // 2. Render Fullscreen Raymarching Shader Pass
 *         Shader* raymarchShader = gfx.getShader("Raymarch");
 *         if (raymarchShader) {
 *             raymarchShader->use();
 *             raymarchShader->setFloat("u_Time", getTime());
 *             gfx.drawFullscreenQuad("Raymarch");
 *         }
 *
 *         // 3. Render 2D UI overlay using built-in default primitives
 *         gfx.drawRectangle(glm::vec2(10, 10), glm::vec2(150, 40), glm::vec4(0, 0, 0, 0.5f));
 *         gfx.drawCircle(glm::vec2(30, 30), 10.0f, glm::vec4(0, 1, 0, 1));
 *
 *         // 4. Safe state unbind at frame end
 *         gfx.unbindAll();
 *     }
 * }
 * @endcode
 */

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "QuadRenderer.hpp"
#include "Primitives2D.hpp"
#include "Texture.hpp"

/**
 * @class GraphicsManager
 * @brief High-level orchestration unit. Handles lifecycle of shaders, primitive batches, and state sanity checks.
 */
class GraphicsManager {
public:
    GraphicsManager(int width, int height);
    ~GraphicsManager() = default;

    GraphicsManager(const GraphicsManager&) = delete;
    GraphicsManager& operator=(const GraphicsManager&) = delete;

    /**
     * @brief Resizes window
     */
    void updateViewport(int newWidth, int newHeight);

    // Resource registration

    Shader* createShaderFromSource(const std::string& name, const std::string& vert, const std::string& frag, const std::string& geom = "");
    Shader* createShaderFromFiles(const std::string& name, const std::filesystem::path& vertPath, const std::filesystem::path& fragPath, const std::filesystem::path& geomPath = "");
    Shader* getShader(const std::string& name);

    // Drawing operations
    void drawFullscreenQuad(const std::string& shaderName);
    void drawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float thickness = 1.0f);
    void drawRectangle(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    void drawCircle(const glm::vec2& center, float radius, const glm::vec4& color);

    // Texture Display Utilities
    void drawTexture(const Texture& texture, const glm::vec2& pos, const glm::vec2& size);
    void drawTextureFullscreen(const Texture& texture);

    // Babying the state engine: enforces state cleanup when required
    void unbindAll();

private:
    int screenWidth;
    int screenHeight;
    glm::mat4 orthoProjection;

    std::unordered_map<std::string, std::unique_ptr<Shader>> shaderRegistry;
    std::unique_ptr<QuadRenderer> fullscreenQuad;
    std::unique_ptr<Primitives2D> primitives2D;

    void compileDefaultShaders();
};