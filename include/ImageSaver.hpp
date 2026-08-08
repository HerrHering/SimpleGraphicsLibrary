/**
 * @file ImageSaver.hpp
 * @brief Utility namespace for saving CPU pixel buffers and GPU Textures to PNG files.
 *
 * @details
 * FEATURES & OPENGL DETAILS:
 * - Supports 1-channel (grayscale GL_RED), 2-channel (GL_RG), 3-channel (GL_RGB), and 4-channel (GL_RGBA) images.
 * - Automatic nanosecond timestamping (YYYY-MM-DD_HH-MM-SS_nanoseconds) prevents accidental file overwrites.
 * - Enforces glPixelStorei(GL_PACK_ALIGNMENT, 1) during GPU downloads so 1-channel images aren't corrupted by OpenGL 4-byte row padding.
 * - Automatically flips OpenGL's Bottom-Left origin vertically to match standard Top-Left PNG orientation.
 *
 * @example Usage Example:
 * @code
 * #include "ImageSaver.hpp"
 * 
 * // 1. Save a 1-channel grayscale CPU vector with timestamp
 * std::vector<uint8_t> grayPixels(256 * 256, 128);
 * ImageSaver::saveFromBuffer("output/noise.png", grayPixels, 256, 256, { .appendTimestamp = true, .channels = 1 });
 * 
 * // 2. Save directly from a custom Texture GPU handle
 * ImageSaver::saveFromTexture("output/render.png", myTexture, { .appendTimestamp = true, .channels = 1 });
 * @endcode
 */

#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

class Texture;

namespace ImageSaver {

    struct Options {
        bool appendTimestamp = true; // Appends YYYY-MM-DD_HH-MM-SS_nanoseconds to filename
        int channels = 1;            // 1 = Grayscale (GL_RED), 3 = RGB, 4 = RGBA
        bool flipVertically = true;  // Corrects OpenGL bottom-left origin to standard top-left PNG
    };

    /**
     * @brief Formats a high-precision nanosecond timestamp string (YYYY-MM-DD_HH-MM-SS_nanoseconds).
     */
    std::string generateTimestamp();

    /**
     * @brief Helper 1: Saves a raw CPU pixel vector to disk as a PNG.
     * @param filepath Target output path (e.g., "output/single_channel.png")
     * @param pixels Raw uint8_t byte array
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @param options Configuration flags (channels, timestamp, vertical flip)
     * @return true if write succeeded, false otherwise
     */
    bool saveFromBuffer(const std::filesystem::path& filepath, 
                        const std::vector<uint8_t>& pixels, 
                        int width, 
                        int height, 
                        Options options = {});

    /**
     * @brief Helper 2: Downloads pixels from a custom GPU Texture handle and saves it to disk as a PNG.
     * @param filepath Target output path
     * @param texture Custom Texture handle reference
     * @param options Configuration flags (channels, timestamp, vertical flip)
     * @return true if download and write succeeded, false otherwise
     */
    bool saveFromTexture(const std::filesystem::path& filepath, 
                         const Texture& texture, 
                         Options options = {});
}