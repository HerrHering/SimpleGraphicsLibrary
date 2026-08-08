#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Logger.hpp"
#include "GraphicsManager.hpp"
#include "OpenGLDebug.hpp"
#include "Camera.hpp"
#include "SDFScene.hpp"

#include <iostream>
#include <memory>

struct AppContext {
    Camera camera;
    SDFScene scene;
    bool pendingRecompile = true;
    
    int width = 1280;
    int height = 720;
    float lastX = 640.0f;
    float lastY = 360.0f;
    bool firstMouse = true;
};

void processInput(GLFWwindow* window, AppContext& ctx, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) ctx.camera.move(Camera::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) ctx.camera.move(Camera::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) ctx.camera.move(Camera::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) ctx.camera.move(Camera::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) ctx.camera.move(Camera::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ctx.camera.move(Camera::DOWN, deltaTime);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; (void)mods;
    auto* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (!ctx || action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_1) { ctx->scene.currentPrimitive = PrimitiveType::SPHERE; ctx->pendingRecompile = true; }
    if (key == GLFW_KEY_2) { ctx->scene.currentPrimitive = PrimitiveType::BOX;    ctx->pendingRecompile = true; }
    if (key == GLFW_KEY_3) { ctx->scene.currentPrimitive = PrimitiveType::TORUS;  ctx->pendingRecompile = true; }
}

void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    auto* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (!ctx) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (ctx->firstMouse) {
        ctx->lastX = xpos;
        ctx->lastY = ypos;
        ctx->firstMouse = false;
    }

    ctx->camera.rotate(xpos - ctx->lastX, ctx->lastY - ypos);
    ctx->lastX = xpos;
    ctx->lastY = ypos;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (ctx) {
        ctx->width = width;
        ctx->height = height;
    }
}

int main() {
    // Hijack std::cerr with a minimum log threshold level
    // Levels: LogSeverity::Verbose, Info, Warning, Error, Fatal
    Logger logger(LogSeverity::Info);

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    auto ctx = std::make_unique<AppContext>();
    GLFWwindow* window = glfwCreateWindow(ctx->width, ctx->height, "Modern OpenGL Graphics Framework", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, ctx.get());
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (gladLoadGL(glfwGetProcAddress) == 0) return -1;

    setupOpenGLDebugging();

    // Instantiate Central Manager
    GraphicsManager gfx(ctx->width, ctx->height);

    float lastFrameTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        processInput(window, *ctx, deltaTime);
        gfx.updateViewport(ctx->width, ctx->height);

        // Dynamic hot-reload/compilation of Raymarch Fragment Shader
        if (ctx->pendingRecompile) {
            std::cout << "[System] Compiling Dynamic Pipeline Shader...\n";
            gfx.createShaderFromSource("Raymarch", SDFScene::getDefaultVertexShaderSource(), ctx->scene.getFragmentShaderSource());
            ctx->pendingRecompile = false;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 1. Raymarching Shader Render Pass
        Shader* raymarchShader = gfx.getShader("Raymarch");
        if (raymarchShader) {
            raymarchShader->use();
            glm::mat4 invView = glm::inverse(ctx->camera.getViewMatrix());
            raymarchShader->setVec2("u_Resolution", glm::vec2(ctx->width, ctx->height));
            raymarchShader->setMat4("u_InvView", invView);
            raymarchShader->setFloat("u_Time", currentFrameTime);

            gfx.drawFullscreenQuad("Raymarch");
        }

        // 2. Overlay 2D Primitive Render Pass (Raylib-Style Screen Coordinates)
        // Draw UI elements over 3D render:
        gfx.drawRectangle(glm::vec2(20, 20), glm::vec2(200, 30), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f)); // Status box
        gfx.drawCircle(glm::vec2(50, 35), 8.0f, glm::vec4(0.0f, 1.0f, 0.2f, 1.0f));                  // Status LED indicator
        
        // Reticle / Crosshair in screen-center:
        glm::vec2 center(ctx->width / 2.0f, ctx->height / 2.0f);
        gfx.drawLine(center - glm::vec2(10, 0), center + glm::vec2(10, 0), glm::vec4(1.0f), 2.0f);
        gfx.drawLine(center - glm::vec2(0, 10), center + glm::vec2(0, 10), glm::vec4(1.0f), 2.0f);

        // Safely unbind context states after frame loop
        gfx.unbindAll();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}