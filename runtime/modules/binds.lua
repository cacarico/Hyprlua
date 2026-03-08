--- Binds Module
--- Handles keybinding registration via the Hyprland API.
--- @module "binds"

local M = {}

--- Registers a keybind with Hyprland.
--- @param mods string: Modifier keys (e.g. "SUPER", "SUPER SHIFT")
--- @param key string: Key name (e.g. "Return", "h")
--- @param dispatcher string: Hyprland dispatcher (e.g. "exec", "movefocus")
--- @param args string: Arguments for the dispatcher (e.g. "kitty", "l")
--- @param opts table|nil: Optional table with fields:
---   - flags (string): l=locked, r=release, e=repeat, m=mouse, n=nonConsuming, t=transparent, i=ignoreMods
---   - submap (string): Register bind in a named submap instead of the current one
function M.set(mods, key, dispatcher, args, opts)
	assert(type(mods) == "string", "mods must be a string")
	assert(type(key) == "string", "key must be a string")
	assert(type(dispatcher) == "string", "dispatcher must be a string")
	assert(type(args) == "string", "args must be a string")
	assert(opts == nil or type(opts) == "table", "opts must be a table or nil")

	local flags = ""
	local submap = ""

	if opts then
		if opts.flags then
			assert(type(opts.flags) == "string", "opts.flags must be a string")
			flags = opts.flags
		end
		if opts.submap then
			assert(type(opts.submap) == "string", "opts.submap must be a string")
			submap = opts.submap
		end
	end

	-- luacheck: push ignore 113
	if __hypr_add_bind then
		__hypr_add_bind(mods, key, dispatcher, args, flags, submap)
		-- luacheck: pop
	else
		error("__hypr_add_bind is not defined in Lua runtime")
	end
end

--- Register binds within a named submap.
--- All binds.set() calls inside fn() automatically get { submap = name }.
--- A catchall reset is appended automatically.
--- @param name string: The submap name
--- @param fn function: Callback that registers binds (receives a bind helper)
function M.submap(name, fn)
	assert(type(name) == "string", "submap name must be a string")
	assert(type(fn) == "function", "submap body must be a function")

	local sub = { submap = name }

	--- Helper: register a bind in this submap.
	--- @param mods string
	--- @param key string
	--- @param dispatcher string
	--- @param args string
	--- @param opts table|nil: extra opts (flags are merged, submap is forced)
	local function bind(mods, key, dispatcher, args, opts)
		local merged = { submap = name }
		if opts then
			for k, v in pairs(opts) do
				merged[k] = v
			end
			merged.submap = name -- force submap
		end
		M.set(mods, key, dispatcher, args, merged)
	end

	fn(bind)

	-- Auto-append catchall reset
	M.set("", "catchall", "submap", "reset", sub)
end

-- luacheck: push ignore 112
hypr.binds = M
-- luacheck: pop
return M
