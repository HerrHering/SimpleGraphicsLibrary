#include "ImageSaver.hpp"
#include "Texture.hpp"
#include "Logger.hpp"

#include <glad/gl.h>
#include <chrono>
#include <iomanip>
#include <sstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace ImageSaver {

    std::string generateTimestamp() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto time_c = system_clock::to_time_t(now);
        
        std::tm tm_buf{};
        #if defined(_WIN32)
        localtime_s(&tm_buf, &time_c);
        #else
        localtime_r(&time_c, &tm_buf);
        #endif

        // Calculate nanosecond remainder
        auto nanosecs = duration_cast<nanoseconds>(now.time_since_epoch() % seconds(1)).count();

        std::ostringstream ss;
        ss << std::put_time(&tm_buf, "%Y-%m-%d_%H-%M-%S_")
           << std::setfill('0') << std::setw(9) << nanosecs;
        return ss.str();
    }

    static std::filesystem::path resolveFilePath(const std::filesystem::path& filepath, bool appendTimestamp) {
        if (!appendTimestamp) return filepath;

        std::filesystem::path dir = filepath.parent_path();
        std::string stem = filepath.stem().string();
        std::string ext = filepath.extension().string();

        if (ext.empty()) ext = ".png";

        std::string timestampedName = stem + "_" + generateTimestamp() + ext;
        return dir.empty() ? std::filesystem::path(timestampedName) : dir / timestampedName;
    }

    bool saveFromBuffer(const std::filesystem::path& filepath, 
                        const std::vector<uint8_t>& pixels, 
                        int width, 
                        int height, 
                        Options options) {
        if (pixels.empty() || width <= 0 || height <= 0) {
            LOG_ERROR << "[ImageSaver] Invalid buffer or image dimensions (" << width << "x" << height << ").\n";
            return false;
        }

        if (options.channels < 1 || options.channels > 4) {
            LOG_ERROR << "[ImageSaver] Invalid channel count: " << options.channels << " (Must be 1, 2, 3, or 4).\n";
            return false;
        }

        size_t expectedSize = static_cast<size_t>(width) * height * options.channels;
        if (pixels.size() < expectedSize) {
            LOG_ERROR << "[ImageSaver] Buffer size (" << pixels.size() << " bytes) is smaller than expected (" << expectedSize << " bytes).\n";
            return false;
        }

        std::filesystem::path finalPath = resolveFilePath(filepath, options.appendTimestamp);

        // Ensure target directory exists
        if (!finalPath.parent_path().empty()) {
            std::filesystem::create_directories(finalPath.parent_path());
        }

        // Configure vertical flip to translate OpenGL bottom-left origin to top-left PNG standard
        stbi_flip_vertically_on_write(options.flipVertically ? 1 : 0);

        int stride = width * options.channels;
        int result = stbi_write_png(finalPath.string().c_str(), width, height, options.channels, pixels.data(), stride);

        if (result) {
            LOG_INFO << "[ImageSaver] Saved image (" << width << "x" << height << ", " << options.channels << " ch) -> " << finalPath.string() << "\n";
            return true;
        } else {
            LOG_ERROR << "[ImageSaver] Failed to write PNG file: " << finalPath.string() << "\n";
            return false;
        }
    }

    bool saveFromTexture(const std::filesystem::path& filepath, 
                         const Texture& texture, 
                         Options options) {
        if (texture.id == 0 || texture.width <= 0 || texture.height <= 0) {
            LOG_ERROR << "[ImageSaver] Cannot download uninitialized texture (ID: " << texture.id << ").\n";
            return false;
        }

        // Map requested channels to OpenGL format
        GLenum glFormat = GL_RGBA;
        if (options.channels == 1)      glFormat = GL_RED;
        else if (options.channels == 2) glFormat = GL_RG;
        else if (options.channels == 3) glFormat = GL_RGB;
        else if (options.channels == 4) glFormat = GL_RGBA;

        std::vector<uint8_t> downloadedPixels(static_cast<size_t>(texture.width) * texture.height * options.channels);

        glBindTexture(GL_TEXTURE_2D, texture.id);

        // CRITICAL OPENGL DETAIL: Set pack alignment to 1 byte!
        // By default, OpenGL assumes texture rows are aligned to 4 bytes.
        // For 1-channel (GL_RED) images, non-4-byte row widths will corrupt row strides if alignment isn't set to 1!
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        
        // Read GPU VRAM pixels back into CPU buffer
        glGetTexImage(GL_TEXTURE_2D, 0, glFormat, GL_UNSIGNED_BYTE, downloadedPixels.data());
        
        glPixelStorei(GL_PACK_ALIGNMENT, 4); // Restore GL default alignment

        glBindTexture(GL_TEXTURE_2D, 0);

        return saveFromBuffer(filepath, downloadedPixels, texture.width, texture.height, options);
    }
}