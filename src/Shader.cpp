#include "Shader.hpp"
#include "ShaderLoader.hpp"
#include <GLFW/glfw3.h> // Included for glfwGetCurrentContext()
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

Shader::~Shader() {
    cleanup();
}

Shader::Shader(Shader&& other) noexcept : id(other.id), uniformCache(std::move(other.uniformCache)), missingUniforms(std::move(other.missingUniforms)) {
    other.id = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        cleanup();
        id = other.id;
        uniformCache = std::move(other.uniformCache);
        missingUniforms = std::move(other.missingUniforms);
        other.id = 0;
    }
    return *this;
}

void Shader::cleanup() {
    if (id != 0) {
        if (glfwGetCurrentContext() != nullptr) { // If context was deleted, these glDelete functions will cause SegFaults
            glDeleteProgram(id);
        }
        id = 0;
    }
    uniformCache.clear();
    missingUniforms.clear();
}

void Shader::use() const {
    if (id != 0) {
        glUseProgram(id);
    }
}

bool Shader::compileFromSource(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc) {
    if (vertexSrc.empty() || fragmentSrc.empty()) {
        std::cerr << "[Shader Error] Empty shader sources provided for compilation.\n";
        return false;
    }

    const char* vCode = vertexSrc.c_str();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vCode, nullptr);
    glCompileShader(vertexShader);
    if (!checkCompileErrors(vertexShader, "VERTEX", vertexSrc)) {
        glDeleteShader(vertexShader);
        return false;
    }

    const char* fCode = fragmentSrc.c_str();
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fCode, nullptr);
    glCompileShader(fragmentShader);
    if (!checkCompileErrors(fragmentShader, "FRAGMENT", fragmentSrc)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    GLuint geometryShader = 0;
    if (!geometrySrc.empty()) {
        const char* gCode = geometrySrc.c_str();
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &gCode, nullptr);
        glCompileShader(geometryShader);
        if (!checkCompileErrors(geometryShader, "GEOMETRY", geometrySrc)) {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteShader(geometryShader);
            return false;
        }
    }

    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertexShader);
    glAttachShader(newProgram, fragmentShader);
    if (geometryShader != 0) {
        glAttachShader(newProgram, geometryShader);
    }
    
    glLinkProgram(newProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShader != 0) glDeleteShader(geometryShader);

    if (!checkCompileErrors(newProgram, "PROGRAM")) {
        glDeleteProgram(newProgram);
        return false;
    }

    cleanup();
    id = newProgram;
    return true;
}

bool Shader::compileFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath, const std::filesystem::path& geometryPath) {
    auto vOpt = ShaderLoader::loadFile(vertexPath);
    auto fOpt = ShaderLoader::loadFile(fragmentPath);

    if (!vOpt || !fOpt) return false;

    std::string gSrc = "";
    if (!geometryPath.empty()) {
        auto gOpt = ShaderLoader::loadFile(geometryPath);
        if (gOpt) gSrc = *gOpt;
        else return false;
    }

    return compileFromSource(*vOpt, *fOpt, gSrc);
}

GLint Shader::getUniformLocation(const std::string& name) const {
    if (const auto it = uniformCache.find(name); it != uniformCache.cend()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(id, name.c_str());
    if (location == -1) {
        if (missingUniforms.find(name) == missingUniforms.end()) {
            std::cerr << "[Shader Warning] Uniform '" << name << "' missing or optimized out in Program ID: " << id << "\n";
            missingUniforms.insert(name);
        }
    } else {
        uniformCache[name] = location;
    }
    return location;
}

void Shader::setBool(const std::string& name, bool value) const { glUniform1i(getUniformLocation(name), static_cast<int>(value)); }
void Shader::setInt(const std::string& name, int value) const { glUniform1i(getUniformLocation(name), value); }
void Shader::setFloat(const std::string& name, float value) const { glUniform1f(getUniformLocation(name), value); }
void Shader::setVec2(const std::string& name, const glm::vec2& value) const { glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::setVec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::setVec4(const std::string& name, const glm::vec4& value) const { glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::setMat4(const std::string& name, const glm::mat4& value) const { glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)); }

void Shader::printSourceWithLineNumbers(const std::string& source) {
    std::stringstream ss(source);
    std::string line;
    int lineNum = 1;
    std::cerr << "--- Source Code Dump ---\n";
    while (std::getline(ss, line)) {
        std::cerr << std::setw(4) << lineNum++ << " | " << line << "\n";
    }
    std::cerr << "------------------------\n";
}

bool Shader::checkCompileErrors(GLuint shader, const std::string& type, const std::string& source) {
    GLint success;
    GLchar infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "\n========================================"
                      << "\nGLSL COMPILATION ERROR [" << type << "]:\n" << infoLog << "\n";
            if (!source.empty()) {
                printSourceWithLineNumbers(source);
            }
            std::cerr << "========================================\n\n";
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "\n========================================"
                      << "\nGLSL LINKING ERROR [PROGRAM]:\n" << infoLog
                      << "\n========================================\n\n";
            return false;
        }
    }
    return true;
}