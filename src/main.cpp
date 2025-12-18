#include "globals.hpp"
#include "watcher.hpp"
#include "utils.hpp"
#include "lua/runtime.hpp"

#include <filesystem>
#include <string>
#include <stdexcept>
#include <iostream> // Logging
#include <cstdlib>  // getenv()

/**
 * @file main.cpp
 * @brief Main implementation of Hyprlua plugin for Hyprland compositor
 * @details Contains plugin initialization/teardown logic and core functionality
 */

/// @brief Color for error notifications (red)
const CHyprColor ERROR_COLOR = {1.0, 0.2, 0.2, 1.0};
/// @brief Color for success notifications (blue)
const CHyprColor SUCCESS_COLOR = {0.2, 0.6, 1.0, 1.0};
/// @brief Display duration for error notifications (5 seconds)
const int ERROR_TIMEOUT = 5000;
/// @brief Display duration for success notifications (3 seconds)
const int SUCCESS_TIMEOUT = 3000;

/**
 * @brief Get plugin API version
 * @return HYPRLAND_API_VERSION as defined by Hyprland
 * @note Required entry point for Hyprland plugins
 */
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

/**
 * @brief Initialize the Hyprlua plugin
 * @param handle Hyprland API handle provided by the compositor
 * @return PLUGIN_DESCRIPTION_INFO containing plugin metadata
 * @throws std::runtime_error On version mismatch or initialization failure
 * @details Performs:
 * 1. API version validation
 * 2. Environment configuration parsing
 * 3. File watcher initialization
 * 4. Initial notification setup
 *
 * @note Uses HYPRLUA_CONFIG_PATH environment variable for config location
 */
APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    try {
        PHANDLE                = handle;
        const std::string HASH = __hyprland_api_get_hash();

        // // Validate API compatibility
        // if (HASH != GIT_COMMIT_HASH) {
        //     sendNotification("[Hyprlua] Mismatched headers! Can't proceed.", ERROR_COLOR, ERROR_TIMEOUT);
        //     throw std::runtime_error("[Hyprlua] Version mismatch");
        // }

        // Handle config path resolution
        const char* configPathEnv = std::getenv("HYPRLUA_CONFIG_PATH");
        std::string filepath      = configPathEnv ? configPathEnv : "~/.config/hypr/hyprland.lua";
        filepath                  = expandTilde(filepath);

        // Handle module path resolution
        const char* modulePathEnv = std::getenv("HYPRLUA_MODULES_PATH");
        std::string modulePath      = modulePathEnv ? modulePathEnv : "/usr/share/hyprlua/modules";
        modulePath                  = expandTilde(modulePath);

        // Set up filesystem monitoring
        std::filesystem::path filePathObj(filepath);
        const std::string     directory = filePathObj.parent_path().string();

        // Initialize file watcher
        g_FileWatcher = std::make_unique<FileWatcher>(filepath, directory);
        if (!g_FileWatcher) {
            throw std::runtime_error("[Hyprlua] Failed to allocate FileWatcher");
        }

        g_FileWatcher->start();
        sendNotification("[Hyprlua] Plugin initialized successfully.", SUCCESS_COLOR, SUCCESS_TIMEOUT);

        hyprlua::init_lua_runtime(modulePath, filepath);

        return {"Hyprlua", "A plugin to enable Lua support for Hyprland", "cacarico", "0.1"};
    } catch (const std::exception& e) {
        std::cerr << "[Hyprlua] Initialization error: " << e.what() << std::endl;
        return {"Hyprlua", "Initialization failed", "cacarico", "0.1"};
    }
}

/**
 * @brief Clean up plugin resources
 * @details Performs:
 * 1. File watcher termination
 * 2. Resource deallocation
 * 3. Final status notification
 * @note Guarantees safe shutdown even if exceptions occur
 */
APICALL EXPORT void PLUGIN_EXIT() {
    try {
        if (g_FileWatcher) {
            g_FileWatcher->stop();
            g_FileWatcher.reset();
        }

        sendNotification("[Hyprlua] Plugin exiting. Stopped file monitoring.", SUCCESS_COLOR, SUCCESS_TIMEOUT);
    } catch (const std::exception& e) { std::cerr << "[Hyprlua] Error during exit: " << e.what() << std::endl; }
}
