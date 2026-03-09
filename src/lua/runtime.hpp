/**
 * @file runtime.hpp
 * @brief Lua runtime lifecycle (init, reload, state access)
 */
#pragma once

#include <string>

namespace hyprlua {

    /// @param modules_path Directory containing Lua wrapper modules
    /// @param user_config_path Path to the user's hyprland.lua
    void init_lua_runtime(const std::string& modules_path, const std::string& user_config_path);

    /// Clears keybinds, resets sol2 state, and reinitializes
    void reload_lua_runtime();

} // namespace hyprlua
