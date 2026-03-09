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
#include <execinfo.h>
#include <signal.h>
#include <cstring>

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

    inline void crash_handler(int sig) {
        void*  frames[64];
        int    n    = backtrace(frames, 64);
        char** syms = backtrace_symbols(frames, n);

        write("FATAL", "Caught signal " + std::to_string(sig) + " (" + strsignal(sig) + ")");
        if (syms) {
            for (int i = 0; i < n; ++i)
                write("FATAL", "  " + std::string(syms[i]));
            free(syms);
        }
        // Re-raise to let the default handler produce core dump / Hyprland secure mode
        signal(sig, SIG_DFL);
        raise(sig);
    }

    inline void install_crash_handler() {
        signal(SIGSEGV, crash_handler);
        signal(SIGABRT, crash_handler);
        signal(SIGBUS,  crash_handler);
        info("Crash handler installed");
    }

} // namespace hyprlua::log
