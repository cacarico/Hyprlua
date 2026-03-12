#include "runtime.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/helpers/Color.hpp>
#include <sol/sol.hpp>
#include <filesystem>
#include <mutex>
#include <thread>
#include "logger.hpp"

#include "lua/monitors.hpp"
#include "lua/keybinds.hpp"
#include "lua/exec.hpp"
#include "utils.hpp"

namespace hyprlua {

    namespace fs = std::filesystem;

    static sol::state  lua;
    static bool        initialized = false;
    static std::string s_modules_path;
    static std::string s_user_config_path;
    static std::mutex  lua_mutex;

    /* Must be called with lua_mutex held */
    static void init_lua_runtime_locked() {
        log::info("Initializing Lua runtime...");

        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::table, sol::lib::string, sol::lib::os);

        // Restrict dangerous os functions. User config is trusted; these are removed to
        // limit accidental misuse (exec/exec_once are the intentional process-spawning API).
        lua["os"]["execute"] = sol::nil;
        lua["os"]["remove"]  = sol::nil;
        lua["os"]["rename"]  = sol::nil;
        lua["os"]["exit"]    = sol::nil;

        // Restrict package library — prevent loading arbitrary native .so files via require().
        lua["package"]["loadlib"] = sol::nil;
        lua["package"]["cpath"]   = "";

        hyprlua::modules::bind_monitors(lua);
        hyprlua::modules::bind_keybinds(lua);
        hyprlua::modules::bind_exec(lua);

        auto names = modules::list_monitors();
        log::info("Hyprland reports these monitors:");
        for (auto& n : names) {
            log::info("  • " + n);
        }

        /* hypr global table — Lua modules attach themselves here (e.g. hypr.monitors) */
        lua["hypr"]            = lua.create_table();
        lua["hypr"]["version"] = HYPRLUA_VERSION;

        try {
            for (const auto& script : {"monitors.lua", "binds.lua", "exec.lua"}) {
                std::string script_path = s_modules_path + "/" + script;
                if (!fs::exists(script_path)) {
                    sendNotification("Module not found: " + script_path, CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
                    log::error("Missing module: " + script_path);
                    continue;
                }
                sol::protected_function_result res = lua.safe_script_file(script_path);
                if (!res.valid()) {
                    sol::error err = res;
                    log::error("Error loading module " + std::string(script) + ": " + err.what());
                    sendNotification("Error loading module: " + std::string(script), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
                }
            }

            // Runtime is structurally ready once modules have been processed.
            // Set before user config so retries are possible even if user config has errors.
            initialized = true;

            if (!fs::exists(s_user_config_path)) {
                sendNotification("Cant find: " + s_user_config_path, CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
                log::error("Config not found: " + s_user_config_path);
                return;
            }

            sol::protected_function_result result = lua.safe_script_file(s_user_config_path);
            if (!result.valid()) {
                sol::error err = result;
                log::error("Error executing config: " + std::string(err.what()));
                sendNotification("Error executing: " + s_user_config_path, CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
                // Do not mark startup done — exec_once should retry on the next
                // successful load rather than being permanently suppressed.
                return;
            }

            hyprlua::modules::mark_startup_done();

        } catch (const std::exception& e) {
            log::error("Runtime initialization failed: " + std::string(e.what()));
            // initialized remains false so a subsequent reload can retry
        }
    }

    void init_lua_runtime(const std::string& modules_path, const std::string& user_config_path) {
        std::lock_guard<std::mutex> lock(lua_mutex);

        s_modules_path     = modules_path;
        s_user_config_path = user_config_path;

        log::info("init_lua_runtime: called, initialized=" + std::to_string(initialized));

        if (initialized) {
            log::info("Lua runtime already initialized, skipping");
            return;
        }

        init_lua_runtime_locked();
    }

    void reload_lua_runtime() {
        std::lock_guard<std::mutex> lock(lua_mutex);

        log::info("reload_lua_runtime: START thread=" +
                  std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));
        hyprlua::modules::clear_plugin_binds();
        initialized = false;
        lua         = sol::state{};
        init_lua_runtime_locked();
    }

} // namespace hyprlua
