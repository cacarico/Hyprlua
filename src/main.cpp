#include "globals.hpp"
#include "watcher.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "lua/runtime.hpp"

#include <hyprland/src/event/EventBus.hpp>

#include <filesystem>
#include <string>
#include <cstdlib>

/**
 * @file main.cpp
 * @brief Hyprlua plugin entry points (init, exit, version)
 */

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

        const char*       configPathEnv = std::getenv("HYPRLUA_CONFIG_PATH");
        std::string       filepath      = configPathEnv ? configPathEnv : "~/.config/hypr/hyprland.lua";
        filepath                        = expandTilde(filepath);

        const char* modulePathEnv = std::getenv("HYPRLUA_MODULES_PATH");
        std::string modulePath    = modulePathEnv ? modulePathEnv : "/usr/share/hyprlua/modules";
        modulePath                = expandTilde(modulePath);

        std::filesystem::path filePathObj(filepath);
        const std::string     directory = filePathObj.parent_path().string();

        hyprlua::init_lua_runtime(modulePath, filepath);

        g_FileWatcher = std::make_unique<FileWatcher>(filepath, directory);
        g_FileWatcher->start();

        /* hyprctl reload clears all keybinds including plugin-added ones */
        static auto s_configReloadedListener = Event::bus()->m_events.config.reloaded.listen([] {
            hyprlua::log::info("Hyprland config reloaded - re-registering Lua keybinds");
            hyprlua::reload_lua_runtime();
        });

        return {"Hyprlua", "A plugin to enable Lua support for Hyprland", "cacarico", "0.1"};
    } catch (const std::exception& e) {
        hyprlua::log::error("Initialization error: " + std::string(e.what()));
        return {"Hyprlua", "Initialization failed", "cacarico", "0.1"};
    }
}

APICALL EXPORT void PLUGIN_EXIT() {
    try {
        if (g_FileWatcher) {
            g_FileWatcher->stop();
            g_FileWatcher.reset();
        }

    } catch (const std::exception& e) { hyprlua::log::error("Error during exit: " + std::string(e.what())); }
}
