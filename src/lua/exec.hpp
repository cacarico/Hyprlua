/**
 * @file exec.hpp
 * @brief Process spawning module (Lua: __hypr_exec, __hypr_exec_once)
 */
#pragma once

#include <sol/sol.hpp>

namespace hyprlua::modules {

    /// @brief Register exec functions into the Lua state
    void bind_exec(sol::state& lua);

    /// @brief Mark startup as done — exec_once will no longer spawn after this
    void mark_startup_done();

} // namespace hyprlua::modules
