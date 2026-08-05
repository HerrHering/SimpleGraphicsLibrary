/**
 * @file Logger.hpp
 * @brief Highlighting stream buffer hijack for std::cerr with ANSI color coding and severity filtering.
 *
 * @details
 * ASSUMPTIONS & BEHAVIOR:
 * - Automatically intercepts all std::cerr output across the app without requiring edits to existing code.
 * - Detects line severity keywords ("error", "warning", "info", "debug") and applies ANSI escape codes.
 * - Automatically enables Windows VT100 console mode on Windows platforms.
 * - Restores std::cerr to its default state upon destruction (RAII).
 *
 * @example Usage Example:
 * @code
 * #include "Logger.hpp"
 * #include <iostream>
 *
 * int main() {
 *     // 1. Initialize Logger on app startup to hijack std::cerr
 *     Logger logger(LogSeverity::Info); // Minimum filter level: Info
 *
 *     // 2. Output messages using normal std::cerr
 *     std::cerr << "[System] Application starting..." << std::endl;  // Cyan (Info)
 *     std::cerr << "[Shader Warning] Uniform missing!" << std::endl;  // Yellow (Warning)
 *     std::cerr << "GLSL COMPILATION ERROR: line 12" << std::endl;    // Red (Error)
 *
 *     // 3. Dynamically alter severity filtering threshold at runtime
 *     Logger::setFilterLevel(LogSeverity::Warning); // Suppresses Info, keeps Warning and Error
 *
 *     return 0;
 * }
 * @endcode
 * 
 * @example To print accurate line numbers
 * @code
 * #include "Logger.hpp"
 *
 * bool Shader::compileFromSource(...) {
 *  if (vertexSrc.empty() || fragmentSrc.empty()) {
 *      LOG_ERROR << "Empty shader sources provided for compilation.\n";
 *      return false;
 *  }
 * } -> // [Shader.cpp:42 (bool Shader::compileFromSource(...))] [Error] Empty shader sources provided for compilation.
 * @endcode
 */

#pragma once
#include <iostream>
#include <streambuf>
#include <string>
#include <algorithm>
#include <atomic>
#include <memory>
#include <source_location>
#include <filesystem>

#if defined(_WIN32)
#include <windows.h>
#endif

/**
 * @enum LogSeverity
 * @brief Represents message importance levels for filtering and colorizing.
 */
enum class LogSeverity {
    Verbose = 0,
    Info    = 1,
    Warning = 2,
    Error   = 3,
    Fatal   = 4
};

/**
 * @class ColorFilterStreamBuf
 * @brief Custom std::streambuf that intercepts std::cerr output, detects severity,
 *        applies ANSI color codes, and enforces a severity threshold filter.
 */
class ColorFilterStreamBuf : public std::streambuf {
private:
    std::streambuf* originalBuf = nullptr;
    std::string lineBuffer;
    LogSeverity activeBlockSeverity = LogSeverity::Info;
    
    inline static std::atomic<LogSeverity> minThreshold{LogSeverity::Info};

public:
    explicit ColorFilterStreamBuf(std::streambuf* orig) : originalBuf(orig) {
        #if defined(_WIN32)
        // Enable Virtual Terminal Processing on Windows consoles for ANSI color codes
        HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
        #endif
    }

    static void setMinSeverity(LogSeverity severity) {
        minThreshold.store(severity);
    }

    static LogSeverity getMinSeverity() {
        return minThreshold.load();
    }

protected:
    int_type overflow(int_type ch) override {
        if (ch == traits_type::eof()) {
            return traits_type::not_eof(ch);
        }

        char c = static_cast<char>(ch);
        if (c == '\n') {
            flushLine();
        } else {
            lineBuffer += c;
        }
        return ch;
    }

    int sync() override {
        if (!lineBuffer.empty()) {
            flushLine();
        }
        return originalBuf ? originalBuf->pubsync() : 0;
    }

private:
    void flushLine() {
        LogSeverity lineSeverity = detectSeverity(lineBuffer);

        // Multi-line error dump handling (e.g., shader line-numbered source dumps or delimiters "======")
        if (lineBuffer.find("===") == 0 || lineBuffer.find("---") == 0 || (lineBuffer.size() > 5 && lineBuffer[4] == '|')) {
            lineSeverity = activeBlockSeverity;
        } else {
            activeBlockSeverity = lineSeverity;
        }

        // Apply Severity Threshold Filter
        if (lineSeverity >= minThreshold.load()) {
            std::string formattedLine = colorizeLine(lineBuffer, lineSeverity) + "\n";
            if (originalBuf) {
                originalBuf->sputn(formattedLine.c_str(), static_cast<std::streamsize>(formattedLine.size()));
            }
        }
        lineBuffer.clear();
    }

    LogSeverity detectSeverity(const std::string& line) {
        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), 
                       [](unsigned char c){ return std::tolower(c); });

        if (lowerLine.find("error") != std::string::npos || 
            lowerLine.find("fatal") != std::string::npos || 
            lowerLine.find("fail") != std::string::npos || 
            lowerLine.find("severity: high") != std::string::npos) {
            return LogSeverity::Error;
        }
        if (lowerLine.find("warning") != std::string::npos || 
            lowerLine.find("warn") != std::string::npos || 
            lowerLine.find("severity: medium") != std::string::npos || 
            lowerLine.find("severity: low") != std::string::npos) {
            return LogSeverity::Warning;
        }
        if (lowerLine.find("debug") != std::string::npos || 
            lowerLine.find("notification") != std::string::npos) {
            return LogSeverity::Verbose;
        }
        return LogSeverity::Info;
    }

    std::string colorizeLine(const std::string& line, LogSeverity severity) {
        const char* colorCode = "\033[0m"; // Reset
        switch (severity) {
            case LogSeverity::Fatal:   colorCode = "\033[1;41;37m"; break; // Bold White on Red BG
            case LogSeverity::Error:   colorCode = "\033[1;31m";    break; // Bold Red
            case LogSeverity::Warning: colorCode = "\033[1;33m";    break; // Bold Yellow
            case LogSeverity::Info:    colorCode = "\033[1;36m";    break; // Bold Cyan
            case LogSeverity::Verbose: colorCode = "\033[90m";      break; // Dark Gray
        }
        return std::string(colorCode) + line + "\033[0m";
    }
};

/**
 * @class Logger
 * @brief RAII Manager for hijacking std::cerr and setting minimum log thresholds.
 */
class Logger {
private:
    std::streambuf* oldCerrBuf = nullptr;
    std::unique_ptr<ColorFilterStreamBuf> customBuf;

public:
    explicit Logger(LogSeverity minLevel = LogSeverity::Info) {
        oldCerrBuf = std::cerr.rdbuf();
        customBuf = std::make_unique<ColorFilterStreamBuf>(oldCerrBuf);
        std::cerr.rdbuf(customBuf.get());
        ColorFilterStreamBuf::setMinSeverity(minLevel);
    }

    ~Logger() {
        if (oldCerrBuf) {
            std::cerr.rdbuf(oldCerrBuf); // Safely restore standard buffer
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static void setFilterLevel(LogSeverity minLevel) {
        ColorFilterStreamBuf::setMinSeverity(minLevel);
    }
};

/**
 * @struct LogStreamHelper
 * @brief Captures C++20 source location and prepends [File:Line (Function)] tags to std::cerr.
 */
struct LogStreamHelper {
    std::source_location loc;
    const char* tag;

    LogStreamHelper(const char* tagStr, std::source_location l = std::source_location::current())
        : loc(l), tag(tagStr) {}

    template <typename T>
    std::ostream& operator<<(const T& val) const {
        std::filesystem::path p(loc.file_name());
        std::cerr << "[" << p.filename().string() << ":" << loc.line() 
                  << " (" << loc.function_name() << ")] " << tag << " ";
        return std::cerr << val;
    }
};

// ==============================================================================
// Source-Location Logging Macros
// ==============================================================================
#define LOG_ERROR LogStreamHelper("[Error]",   std::source_location::current()) << ""
#define LOG_WARN  LogStreamHelper("[Warning]", std::source_location::current()) << ""
#define LOG_INFO  LogStreamHelper("[Info]",    std::source_location::current()) << ""
#define LOG_DEBUG LogStreamHelper("[Debug]",   std::source_location::current()) << ""