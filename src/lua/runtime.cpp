#include "runtime.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/helpers/Color.hpp>
#include <sol/sol.hpp>
#include <iostream>
#include <filesystem>
#include "logger.hpp"

#include "lua/monitors.hpp"
#include "lua/keybinds.hpp"
#include "utils.hpp"

namespace hyprlua {

    namespace fs = std::filesystem;

    static sol::state  lua;
    static bool        initialized = false;
    static std::string s_modules_path;
    static std::string s_user_config_path;

    sol::state&        get_lua_state() {
        return lua;
    }

    void init_lua_runtime(const std::string& modules_path, const std::string& user_config_path) {
        s_modules_path     = modules_path;
        s_user_config_path = user_config_path;

        log::info("init_lua_runtime: called, initialized=" + std::to_string(initialized));

        if (initialized) {
            std::cerr << "[hyprlua] Lua runtime already initialized." << std::endl;
            return;
        }

        std::cout << "[hyprlua] Initializing Lua runtime..." << std::endl;

        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::table, sol::lib::string, sol::lib::os);

        hyprlua::modules::bind_monitors(lua);
        hyprlua::modules::bind_keybinds(lua);

        auto names = modules::list_monitors();
        log::info("Hyprland reports these monitors:");
        for (auto& n : names) {
            log::info("  • " + n);
        }

        /* hypr global table — Lua modules attach themselves here (e.g. hypr.monitors) */
        lua["hypr"]             = lua.create_table();
        lua["hypr"]["version"]  = "0.1.0";
        lua["hypr"]["monitors"] = lua.create_table();

        try {
            for (const auto& script : {"monitors.lua", "binds.lua"}) {
                std::string script_path = modules_path + "/" + script;
                if (!fs::exists(script_path)) {
                    sendNotification("Module not found: " + script_path, CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
                    std::cerr << "[hyprlua] Warning: missing " << script << " at " << script_path << std::endl;
                    continue;
                }
                lua.script_file(script_path);
            }

            if (!fs::exists(user_config_path)) {
                sendNotification("Cant find: " + user_config_path, CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
                std::cerr << "[hyprlua] Error: config not found in " << user_config_path << std::endl;
                return;
            }

            sol::protected_function_result result = lua.safe_script_file(user_config_path);
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "[hyprlua] Error executing config.lua:\n" << err.what() << std::endl;
                sendNotification("Error executing: " + user_config_path, CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
            }

        } catch (const std::exception& e) {
            std::cerr << "[hyprlua] Runtime initialization failed: " << e.what() << std::endl;

            std::ofstream log("/tmp/hyprlua.log", std::ios::app);
            if (log.is_open()) {
                log << "[hyprlua] Exception during runtime initialization: " << e.what() << "\n";
            }
        }

        initialized = true;
    }

    void reload_lua_runtime() {
        log::info("reload_lua_runtime: starting");
        hyprlua::modules::clear_plugin_binds();
        initialized = false;
        lua         = sol::state{};
        init_lua_runtime(s_modules_path, s_user_config_path);
    }

} // namespace hyprlua
