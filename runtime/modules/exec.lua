--- Exec Module
--- Process spawning with session-once and plugin-load-once semantics.
--- Exposes `hypr.exec()`, `hypr.exec_once()`, and `hypr.exec_on_load()` on the hypr global.
--- @module exec

-- luacheck: push ignore 113 112
function hypr.exec(cmd)
	assert(type(cmd) == "string", "exec: cmd must be a string")
	if __hypr_exec then
		__hypr_exec(cmd)
	else
		error("__hypr_exec is not defined in Lua runtime")
	end
end

--- Fires once per Hyprland session, identified by HYPRLAND_INSTANCE_SIGNATURE.
--- Reloading the plugin within the same Hyprland session will NOT re-run this.
function hypr.exec_once(cmd)
	assert(type(cmd) == "string", "exec_once: cmd must be a string")
	if __hypr_exec_once then
		__hypr_exec_once(cmd)
	else
		error("__hypr_exec_once is not defined in Lua runtime")
	end
end

--- Fires once every time the plugin is loaded or reloaded via hyprctl.
--- Does NOT fire on config file hot-reloads.
function hypr.exec_on_load(cmd)
	assert(type(cmd) == "string", "exec_on_load: cmd must be a string")
	if __hypr_exec_on_load then
		__hypr_exec_on_load(cmd)
	else
		error("__hypr_exec_on_load is not defined in Lua runtime")
	end
end
-- luacheck: pop
