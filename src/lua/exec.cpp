#include "lua/exec.hpp"
#include "logger.hpp"

#include <spawn.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <string>

extern char** environ;

namespace hyprlua::modules {

    // Set to false after the first successful config run per plugin load.
    // Never reset on hot-reload, so exec_on_load does not re-fire.
    static std::atomic<bool> g_plugin_load{true};

    static void spawn(const std::string& cmd) {
        const char* shell = getenv("SHELL");
        if (!shell || shell[0] == '\0')
            shell = "sh"; // resolved via PATH, no hardcoded location

        log::info("exec: spawning via " + std::string(shell) + ": " + cmd);

        const char* argv[] = {shell, "-c", cmd.c_str(), nullptr};

        // Reset all caught signals to SIG_DFL and clear the signal mask in the
        // child so the spawned shell does not inherit compositor signal dispositions.
        posix_spawnattr_t attr;
        posix_spawnattr_init(&attr);
        posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETSIGMASK);

        sigset_t empty;
        sigemptyset(&empty);
        posix_spawnattr_setsigmask(&attr, &empty);

        sigset_t all;
        sigfillset(&all);
        posix_spawnattr_setsigdefault(&attr, &all);

        pid_t pid;
        // Hyprland sets SA_NOCLDWAIT in main(), so children are auto-reaped.
        // posix_spawnp searches PATH for the shell binary.
        int ret = posix_spawnp(&pid, shell, nullptr, &attr, const_cast<char* const*>(argv), environ);
        posix_spawnattr_destroy(&attr);

        if (ret != 0) {
            char buf[64];
            strerror_r(ret, buf, sizeof(buf));
            log::error("exec: posix_spawnp failed: " + std::string(buf));
        }
    }

    // Returns true and creates the flag file if this is the first hyprlua run for
    // the current Hyprland session (identified by HYPRLAND_INSTANCE_SIGNATURE).
    // The flag file lives in XDG_RUNTIME_DIR and is cleaned up when the session ends.
    static bool claim_session_start() {
        const char* sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
        if (!sig || sig[0] == '\0') {
            log::error("exec_once: HYPRLAND_INSTANCE_SIGNATURE not set, cannot determine session");
            return false;
        }

        const char* xdg = getenv("XDG_RUNTIME_DIR");
        std::string flag_path = std::string(xdg ? xdg : "/tmp") + "/hyprlua-" + sig + ".once";

        // O_CREAT|O_EXCL is atomic: succeeds only if the file did not exist.
        // If it already exists (plugin reloaded within same Hyprland session), open fails.
        int fd = open(flag_path.c_str(), O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, 0600);
        if (fd < 0)
            return false; // already started this session

        close(fd);
        log::info("exec_once: session flag created at " + flag_path);
        return true;
    }

    void bind_exec(sol::state& lua) {
        lua.set_function("__hypr_exec", [](const std::string& cmd) {
            spawn(cmd);
        });

        lua.set_function("__hypr_exec_once", [](const std::string& cmd) {
            if (claim_session_start())
                spawn(cmd);
            else
                log::info("exec_once: skipping, already ran this session: " + cmd);
        });

        lua.set_function("__hypr_exec_on_load", [](const std::string& cmd) {
            if (g_plugin_load)
                spawn(cmd);
            else
                log::info("exec_on_load: skipping (not plugin load): " + cmd);
        });
    }

    void mark_startup_done() {
        g_plugin_load = false;
        log::info("exec: plugin load phase complete, exec_on_load suppressed until next plugin load");
    }

} // namespace hyprlua::modules
