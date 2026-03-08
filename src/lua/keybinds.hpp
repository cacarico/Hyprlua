/**
 * @file keybinds.hpp
 * @brief Keybind registration module (Lua: __hypr_add_bind)
 */
#pragma once

#include <sol/sol.hpp>

namespace hyprlua::modules {

    /// @brief Register keybind functions into the Lua state
    void bind_keybinds(sol::state& lua);

    /// @brief Remove all keybinds registered by this plugin (used on hot-reload)
    void clear_plugin_binds();

} // namespace hyprlua::modules
