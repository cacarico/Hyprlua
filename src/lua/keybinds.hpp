// keybinds.hpp
#pragma once

#include <sol/sol.hpp>

namespace hyprlua::modules {
    void bind_keybinds(sol::state& lua);
    void clear_plugin_binds();
} // namespace hyprlua::modules
