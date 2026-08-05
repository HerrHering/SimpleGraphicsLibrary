/**
 * @file ShaderLoader.hpp
 * @brief File system reading and source code macro preprocessing utilities.
 *
 * @details
 * ASSUMPTIONS:
 * - Uses std::filesystem (C++17/C++20).
 * - Macro replacement functions expect precise string target matching in shader code.
 *
 * @example Usage Example:
 * @code
 * #include "ShaderLoader.hpp"
 *
 * void loadAndModifyShader() {
 *     // 1. Safely load file string from disk path
 *     auto sourceOpt = ShaderLoader::loadFile("shaders/raymarch.frag");
 *     if (!sourceOpt) return;
 *
 *     // 2. Perform dynamic macro substitution for hot-swapping code fragments
 *     std::string modifiedSource = ShaderLoader::replaceMacro(
 *         *sourceOpt, 
 *         "#define DYNAMIC_SDF_CALL sdSphere(p, 1.0)", 
 *         "#define DYNAMIC_SDF_CALL sdBox(p, vec3(0.5))"
 *     );
 * }
 * @endcode
 */

#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <optional>

/**
 * @class ShaderLoader
 * @brief Utilities for reading, validating, and preprocessing GLSL files.
 */
class ShaderLoader {
public:
    /**
     * @brief Safely reads text content from a target file system path.
     */
    static std::optional<std::string> loadFile(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::cerr << "[ShaderLoader Error] File does not exist at path: " << path << "\n";
            return std::nullopt;
        }

        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[ShaderLoader Error] Unable to open stream for path: " << path << "\n";
            return std::nullopt;
        }

        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    /**
     * @brief Utility for hot-swapping GLSL macros in source code strings dynamically.
     * 
     * Example: ``` replaceMacro(myShaderString, "SDF_NAME", "sdBox(p, 0.1f)") ```
     * @returns Update shader source code
     */
    static std::string replaceMacro(std::string source, std::string_view targetMacro, std::string_view replacement) {
        const size_t pos = source.find(targetMacro);
        if (pos != std::string::npos) {
            source.replace(pos, targetMacro.length(), replacement);
        } else {
            std::cerr << "[ShaderLoader Warning] Macro replacement target '" << targetMacro << "' missing from shader string.\n";
        }
        return source;
    }
};