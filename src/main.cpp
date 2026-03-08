#include "globals.hpp"
#include "watcher.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "lua/runtime.hpp"

#include <hyprland/src/event/EventBus.hpp>

#include <filesystem>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

/**
 * @file main.cpp
 * @brief Hyprlua plugin entry points (init, exit, version)
 */

const CHyprColor ERROR_COLOR   = {1.0, 0.2, 0.2, 1.0};
const CHyprColor SUCCESS_COLOR = {0.2, 0.6, 1.0, 1.0};
const int ERROR_TIMEOUT        = 5000;
const int SUCCESS_TIMEOUT      = 3000;

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

/**
 * @brief Initialize the Hyprlua plugin
 * @param handle Hyprland plugin API handle
 * @return Plugin metadata
 */
APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    try {
        PHANDLE                = handle;
        const std::string HASH = __hyprland_api_get_hash();

        const char* configPathEnv = std::getenv("HYPRLUA_CONFIG_PATH");
        std::string filepath      = configPathEnv ? configPathEnv : "~/.config/hypr/hyprland.lua";
        filepath                  = expandTilde(filepath);

        const char* modulePathEnv = std::getenv("HYPRLUA_MODULES_PATH");
        std::string modulePath      = modulePathEnv ? modulePathEnv : "/usr/share/hyprlua/modules";
        modulePath                  = expandTilde(modulePath);

        std::filesystem::path filePathObj(filepath);
        const std::string     directory = filePathObj.parent_path().string();

        g_FileWatcher = std::make_unique<FileWatcher>(filepath, directory);
        if (!g_FileWatcher) {
            throw std::runtime_error("[Hyprlua] Failed to allocate FileWatcher");
        }

        g_FileWatcher->start();
        sendNotification("[Hyprlua] Plugin initialized successfully.", SUCCESS_COLOR, SUCCESS_TIMEOUT);

        hyprlua::init_lua_runtime(modulePath, filepath);

        /* hyprctl reload clears all keybinds including plugin-added ones */
        static auto s_configReloadedListener = Event::bus()->m_events.config.reloaded.listen([] {
            hyprlua::log::info("Hyprland config reloaded - re-registering Lua keybinds");
            hyprlua::reload_lua_runtime();
        });

        return {"Hyprlua", "A plugin to enable Lua support for Hyprland", "cacarico", "0.1"};
    } catch (const std::exception& e) {
        std::cerr << "[Hyprlua] Initialization error: " << e.what() << std::endl;
        return {"Hyprlua", "Initialization failed", "cacarico", "0.1"};
    }
}

APICALL EXPORT void PLUGIN_EXIT() {
    try {
        if (g_FileWatcher) {
            g_FileWatcher->stop();
            g_FileWatcher.reset();
        }

        sendNotification("[Hyprlua] Plugin exiting. Stopped file monitoring.", SUCCESS_COLOR, SUCCESS_TIMEOUT);
    } catch (const std::exception& e) { std::cerr << "[Hyprlua] Error during exit: " << e.what() << std::endl; }
}
