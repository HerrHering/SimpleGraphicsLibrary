#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Logger.hpp"
#include "GraphicsManager.hpp"
#include "OpenGLDebug.hpp"
#include "Texture.hpp"
#include "PingPongBuffer.hpp"
#include "SDFScene.hpp"

#include <iostream>
#include <vector>
#include <random>

// GLSL Iteration Shader (Conway's Game of Life on GPU)
namespace Shaders {
    constexpr const char* GameOfLifeFrag = R"(
        #version 460 core
        out vec4 FragColor;
        in vec2 v_UV;

        uniform sampler2D u_InputTexture;
        uniform vec2 u_Resolution;

        int getCell(int dx, int dy) {
            vec2 texelSize = 1.0 / u_Resolution;
            vec2 uv = v_UV + vec2(dx, dy) * texelSize;
            return texture(u_InputTexture, uv).r > 0.5 ? 1 : 0;
        }

        void main() {
            int currentState = getCell(0, 0);
            int neighbors = getCell(-1,-1) + getCell(0,-1) + getCell(1,-1) +
                            getCell(-1, 0)                + getCell(1, 0) +
                            getCell(-1, 1) + getCell(0, 1) + getCell(1, 1);

            float nextState = 0.0;
            if (currentState == 1 && (neighbors == 2 || neighbors == 3)) nextState = 1.0;
            else if (currentState == 0 && neighbors == 3) nextState = 1.0;

            FragColor = vec4(vec3(nextState), 1.0);
        }
    )";
}

int main() {
    Logger logger(LogSeverity::Info);

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Texture & Image Displayer Demo", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    if (gladLoadGL(glfwGetProcAddress) == 0) return -1;

    setupOpenGLDebugging();

    {
        GraphicsManager gfx(1280, 720);

        // =====================================================================
        // WORKFLOW 1: CPU Async Pixel Generation (PBO Upload)
        // =====================================================================
        constexpr int cpuTexW = 256, cpuTexH = 256;
        Texture cpuTexture;
        cpuTexture.create(cpuTexW, cpuTexH, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        std::vector<uint8_t> cpuPixels(cpuTexW * cpuTexH * 4);

        // =====================================================================
        // WORKFLOW 2: GPU Iterative Stage Computation (Ping-Pong Buffer)
        // =====================================================================
        constexpr int simW = 256, simH = 256;
        PingPongBuffer pingPong;
        pingPong.init(simW, simH);

        // Create Game of Life GPU shader
        Shader* simShader = gfx.createShaderFromSource("GameOfLife", SDFScene::getDefaultVertexShaderSource(), Shaders::GameOfLifeFrag);

        // Seed initial noise state into Ping-Pong buffer
        std::vector<uint8_t> initNoise(simW * simH * 4);
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(0, 1);
        for (size_t i = 0; i < simW * simH; ++i) {
            uint8_t val = dist(rng) * 255;
            initNoise[i * 4 + 0] = val;
            initNoise[i * 4 + 1] = val;
            initNoise[i * 4 + 2] = val;
            initNoise[i * 4 + 3] = 255;
        }
        // Seed Texture 0
        const_cast<Texture&>(pingPong.getReadTexture()).updatePixels(initNoise.data());

        float time = 0.0f;

        while (!glfwWindowShouldClose(window)) {
            time += 0.016f;

            // 1. CPU Update: Generate dynamic procedural plasma pixels
            for (int y = 0; y < cpuTexH; ++y) {
                for (int x = 0; x < cpuTexW; ++x) {
                    size_t idx = (y * cpuTexW + x) * 4;
                    cpuPixels[idx + 0] = static_cast<uint8_t>((std::sin(x * 0.05f + time) + 1.0f) * 127.5f); // Red
                    cpuPixels[idx + 1] = static_cast<uint8_t>((std::cos(y * 0.05f + time) + 1.0f) * 127.5f); // Green
                    cpuPixels[idx + 2] = 180;                                                                // Blue
                    cpuPixels[idx + 3] = 255;                                                                // Alpha
                }
            }

            // Upload to GPU non-blockingly via PBO DMA transfer
            cpuTexture.updatePixelsAsync(cpuPixels.data(), cpuPixels.size());

            // 2. GPU Step: Execute 2 computation iterations on the GPU in parallel
            if (simShader) {
                QuadRenderer dummyQuad;
                pingPong.step(*simShader, dummyQuad);
                pingPong.step(*simShader, dummyQuad);
            }

            // 3. Render Pass
            glViewport(0, 0, 1280, 720);
            glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Draw Workflow 1 (CPU Async Texture) on left side of screen
            gfx.drawTexture(cpuTexture, glm::vec2(50, 200), glm::vec2(300, 300));

            // Draw Workflow 2 (GPU Ping-Pong Simulation Output) on right side of screen
            gfx.drawTexture(pingPong.getReadTexture(), glm::vec2(400, 200), glm::vec2(300, 300));

            gfx.unbindAll();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}