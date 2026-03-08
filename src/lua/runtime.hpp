// runtime.hpp
#pragma once

#include <string>
#include <sol/sol.hpp>

namespace hyprlua {

void init_lua_runtime(const std::string& config_path, const std::string& user_config_path);
void reload_lua_runtime();
sol::state& get_lua_state();

} // namespace hyprlua
