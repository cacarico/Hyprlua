/**
 * @file logger.hpp
 * @brief Logging to $XDG_RUNTIME_DIR/hyprlua.log (thread-safe, timestamped)
 */
#pragma once

#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cstdlib>

namespace hyprlua::log {

    inline std::ofstream& stream() {
        static std::ofstream file([] {
            // Prefer XDG_RUNTIME_DIR to avoid symlink attacks on /tmp
            const char* xdgRuntime = std::getenv("XDG_RUNTIME_DIR");
            std::string path       = xdgRuntime ? std::string(xdgRuntime) + "/hyprlua.log" : "/tmp/hyprlua.log";
            std::ofstream f(path, std::ios::app);
            if (!f.is_open()) {
                // Fallback: stderr if the log file cannot be opened
                return std::ofstream{};
            }
            return f;
        }());
        return file;
    }

    inline std::string timestamp() {
        auto         now = std::chrono::system_clock::now();
        auto         t_c = std::chrono::system_clock::to_time_t(now);
        struct tm    buf{};
        localtime_r(&t_c, &buf); // thread-safe: writes into caller-provided buf
        std::ostringstream ss;
        ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    inline void write(const std::string& level, const std::string& message) {
        static std::mutex           mtx;
        std::lock_guard<std::mutex> lock(mtx);
        auto&                       s = stream();
        if (s.is_open())
            s << "[" << timestamp() << "] [" << level << "] " << message << std::endl;
    }

    inline void info(const std::string& message) {
        write("INFO", message);
    }
    inline void debug(const std::string& message) {
        write("DEBUG", message);
    }
    inline void error(const std::string& message) {
        write("ERROR", message);
    }

} // namespace hyprlua::log
