/**
 * @file OpenGLDebug.hpp
 * @brief Human-readable OpenGL Debug Output context callback configuration.
 *
 * @details
 * ASSUMPTIONS:
 * - Requires OpenGL 4.3+ Core Context or `GL_KHR_debug` extension enabled.
 * - GLFW context window hint `GLFW_OPENGL_DEBUG_CONTEXT` must be set to `GLFW_TRUE`.
 * - GLAD/OpenGL pointers must be initialized BEFORE calling `setupOpenGLDebugging()`.
 *
 * @example Usage Example:
 * @code
 * #include <glad/gl.h>
 * #include <GLFW/glfw3.h>
 * #include "OpenGLDebug.hpp"
 *
 * int main() {
 *     glfwInit();
 *     glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Step 1: Enable debug context
 *     GLFWwindow* win = glfwCreateWindow(800, 600, "App", nullptr, nullptr);
 *     glfwMakeContextCurrent(win);
 *     gladLoadGL(glfwGetProcAddress);                       // Step 2: Load GLAD functions
 *
 *     setupOpenGLDebugging();                               // Step 3: Register callback
 * }
 * @endcode
 */

#pragma once
#include <glad/gl.h>
#include <iostream>

/**
 * @file OpenGLDebug.hpp
 * @brief Handles human-readable OpenGL debug callback logging.
 */

namespace Internals {

inline const char* debugSourceToString(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API:             return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Window System";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:     return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
        case GL_DEBUG_SOURCE_OTHER:           return "Other";
        default:                              return "Unknown";
    }
}

inline const char* debugTypeToString(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behavior";
        case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
        case GL_DEBUG_TYPE_OTHER:               return "Other";
        default:                                return "Unknown";
    }
}

inline const char* debugSeverityToString(GLenum severity) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         return "High";
        case GL_DEBUG_SEVERITY_MEDIUM:       return "Medium";
        case GL_DEBUG_SEVERITY_LOW:          return "Low";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
        default:                             return "Unknown";
    }
}

inline void openGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, const void* userParam) {
    (void)id; (void)length; (void)userParam;
    
    // Ignore driver notification spam (e.g., buffer dynamic allocations)
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    std::cerr << "\n[OpenGL Debug Output]\n"
              << "  Source:   " << debugSourceToString(source) << "\n"
              << "  Type:     " << debugTypeToString(type) << "\n"
              << "  Severity: " << debugSeverityToString(severity) << "\n"
              << "  Message:  " << message << "\n" << std::endl;
}

} // namespace Internals

inline void setupOpenGLDebugging() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Synchronous callbacks make debugging via breakpoints straightforward
    glDebugMessageCallback(Internals::openGLDebugCallback, nullptr);
    std::cout << "[GraphicsManager] Registered OpenGL Debug Callback Context." << std::endl;
}