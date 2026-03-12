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
#include <unistd.h>

namespace hyprlua::log {

    inline std::ofstream& stream() {
        static std::ofstream file([] {
            // Follow Hyprland's convention: logs go in
            // $XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/
            // alongside hyprland.log. The directory is created by Hyprland at startup.
            const char* xdgRuntime = std::getenv("XDG_RUNTIME_DIR");
            const char* sig        = std::getenv("HYPRLAND_INSTANCE_SIGNATURE");

            std::string path;
            if (xdgRuntime && sig)
                path = std::string(xdgRuntime) + "/hypr/" + sig + "/hyprlua.log";
            else
                path = "/tmp/hyprlua.log"; // fallback when running outside Hyprland

            std::ofstream f(path, std::ios::app);
            if (!f.is_open())
                return std::ofstream{};
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
        // This handler must use only async-signal-safe functions (see signal-safety(7)).
        // malloc, std::string, std::mutex, and backtrace_symbols are NOT safe here —
        // they can deadlock if the signal fires while their internal locks are held.
        void* frames[64];
        int   n = backtrace(frames, 64);

        // backtrace_symbols_fd writes directly to an fd with no heap allocation.
        // STDERR_FILENO is always open and async-signal-safe to write to.
        const char header[] = "hyprlua: fatal signal, backtrace:\n";
        ::write(STDERR_FILENO, header, sizeof(header) - 1);
        backtrace_symbols_fd(frames, n, STDERR_FILENO);

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
