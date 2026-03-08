/**
 * @file monitors.hpp
 * @brief Monitor configuration module (Lua: __hypr_add_monitor, __hypr_disable_monitor)
 */
#pragma once

#include <sol/sol.hpp>
#include <vector>
#include <string>

namespace hyprlua::modules {

    /// @brief List all connected monitor names (including disabled)
    std::vector<std::string> list_monitors();

    /// @brief Register monitor functions into the Lua state
    void bind_monitors(sol::state& lua);

} // namespace hyprlua::modules
