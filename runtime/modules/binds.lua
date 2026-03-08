--- Binds Module
--- Handles keybinding registration via the Hyprland API.
--- @module "binds"

local M = {}

--- Registers a keybind with Hyprland.
--- @param mods string: Modifier keys (e.g. "SUPER", "SUPER SHIFT")
--- @param key string: Key name (e.g. "Return", "h")
--- @param dispatcher string: Hyprland dispatcher (e.g. "exec", "movefocus")
--- @param args string: Arguments for the dispatcher (e.g. "kitty", "l")
--- @param opts table|nil: Optional table with a `flags` string field
---   Supported flags: l=locked, r=release, e=repeat, m=mouse, n=nonConsuming, t=transparent, i=ignoreMods
function M.set(mods, key, dispatcher, args, opts)
	assert(type(mods) == "string", "mods must be a string")
	assert(type(key) == "string", "key must be a string")
	assert(type(dispatcher) == "string", "dispatcher must be a string")
	assert(type(args) == "string", "args must be a string")
	assert(opts == nil or type(opts) == "table", "opts must be a table or nil")

	local flags = ""
	if opts and opts.flags then
		assert(type(opts.flags) == "string", "opts.flags must be a string")
		flags = opts.flags
	end

	-- luacheck: push ignore 113
	if __hypr_add_bind then
		__hypr_add_bind(mods, key, dispatcher, args, flags)
		-- luacheck: pop
	else
		error("__hypr_add_bind is not defined in Lua runtime")
	end
end

-- luacheck: push ignore 112
hypr.binds = M
-- luacheck: pop
return M
