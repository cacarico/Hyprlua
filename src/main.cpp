#include "globals.hpp"
#include "watcher.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "lua/runtime.hpp"

#include <hyprland/src/event/EventBus.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <cstdlib>

/**
 * @file main.cpp
 * @brief Hyprlua plugin entry points (init, exit, version)
 */

// Type-erased storage for the config.reloaded event listener.
// The listener auto-deregisters when its handle is destroyed (shared_ptr refcount → 0).
// Stored at file scope so PLUGIN_EXIT can reset it before the .so is unloaded —
// preventing a dangling call into unloaded code on the next hyprctl reload.
static std::shared_ptr<void> g_configReloadedListener;

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

        // hyprctl reload clears all keybinds including plugin-added ones.
        // Register a listener to re-apply Lua keybinds after each reload.
        // The handle is stored in g_configReloadedListener (file scope) so
        // PLUGIN_EXIT can deregister it before the .so is unloaded.
        {
            auto handle = Event::bus()->m_events.config.reloaded.listen([] {
                hyprlua::log::info("Hyprland config reloaded - re-registering Lua keybinds");
                hyprlua::reload_lua_runtime();
            });
            // Type-erase the handle via shared_ptr custom deleter.
            // When g_configReloadedListener is reset(), the deleter runs and
            // destroys 'h', which deregisters the event listener.
            g_configReloadedListener = std::shared_ptr<void>(nullptr, [h = std::move(handle)](void*) mutable { (void)h; });
        }

        return {"Hyprlua", "A plugin to enable Lua support for Hyprland", "cacarico", HYPRLUA_VERSION};
    } catch (const std::exception& e) {
        hyprlua::log::error("Initialization error: " + std::string(e.what()));
        sendNotification("[Hyprlua] Initialization failed: " + std::string(e.what()), CHyprColor{1.0, 0.2, 0.2, 1.0}, 8000);
        return {"Hyprlua", "Initialization failed", "cacarico", HYPRLUA_VERSION};
    }
}

APICALL EXPORT void PLUGIN_EXIT() {
    try {
        // Deregister the config.reloaded listener first so no callbacks fire
        // into this .so after it begins unloading.
        g_configReloadedListener.reset();

        if (g_FileWatcher) {
            g_FileWatcher->stop();
            g_FileWatcher.reset();
        }

    } catch (const std::exception& e) { hyprlua::log::error("Error during exit: " + std::string(e.what())); }
}
