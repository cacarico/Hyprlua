/**
 * @file logger.hpp
 * @brief Logging to /tmp/hyprlua.log (thread-safe, timestamped)
 */
#pragma once

#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace hyprlua::log {

inline std::ofstream& stream() {
    static std::ofstream file("/tmp/hyprlua.log", std::ios::app);
    return file;
}

inline std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    auto t_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

inline void write(const std::string& level, const std::string& message) {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    stream() << "[" << timestamp() << "] [" << level << "] " << message << std::endl;
}

inline void info(const std::string& message)  { write("INFO", message); }
inline void debug(const std::string& message) { write("DEBUG", message); }
inline void error(const std::string& message) { write("ERROR", message); }

} // namespace hyprlua::log
