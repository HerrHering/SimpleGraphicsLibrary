/**
 * @file Shader.hpp
 * @brief RAII GLSL program wrapper with uniform caching and verbose compile error reports.
 *
 * @details
 * ASSUMPTIONS:
 * - Uniform locations are lazily looked up and cached in a hash map.
 * - Warns ONCE when a uniform name fails to exist in the compiled program or is optimized out by GLSL driver compiler.
 * - Prints line-numbered shader source dumps when compilation fails.
 *
 * @example Usage Example:
 * @code
 * #include "Shader.hpp"
 *
 * void useShaderPipeline() {
 *     Shader shader;
 *     if (shader.compileFromFiles("shaders/vert.glsl", "shaders/frag.glsl")) {
 *         shader.use();
 *         
 *         // Setting uniform values with automatic caching & missing-uniform warning
 *         shader.setVec3("u_Color", glm::vec3(1.0f, 0.5f, 0.2f));
 *         shader.setMat4("u_Model", glm::mat4(1.0f));
 *     }
 * }
 * @endcode
 */

#pragma once
#include <glad/gl.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
#include <filesystem>

/**
 * @class Shader
 * @brief RAII GLSL Shader object containing verbose compilation output and state caching.
 */
class Shader {
public:
    GLuint id = 0;

    Shader() = default;
    ~Shader();

    // Disable copy semantic semantics to protect OpenGL handle lifecycle
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Enable move semantics
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /**
     * @brief Compiles shader, populating the object
     * @returns Returns ```True``` if no errors occured
     */
    bool compileFromSource(const std::string& vertexSrc, 
                           const std::string& fragmentSrc, 
                           const std::string& geometrySrc = "");
    
    /**
     * @brief Compiles shader, populating the object
     * @returns Returns ```True``` if no errors occured
     */
    bool compileFromFiles(const std::filesystem::path& vertexPath, 
                          const std::filesystem::path& fragmentPath, 
                          const std::filesystem::path& geometryPath = "");

    void use() const;

    // Uniform setters with warning feedback for missing locations

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    // The "mutable" fields are modifiable within const functions, as this is not a logical detail of the class

    mutable std::unordered_map<std::string, GLint> uniformCache;
    mutable std::unordered_set<std::string> missingUniforms;

    // Access uniform fields through the cached id-s
    GLint getUniformLocation(const std::string& name) const;
    // Releases shader resources
    void cleanup();
    
    /**
     * @brief Checks if any errors occured after the preceeding shader source compilation step.
     * @returns Returns ```True``` if no errors occured
     */
    static bool checkCompileErrors(GLuint shader, const std::string& type, const std::string& source = "");
    // Debug: A function to nicely visualize the shader source code with line numbers
    static void printSourceWithLineNumbers(const std::string& source);
};