--- Mock helpers for __hypr_* C++ globals.
--- @module "helpers.mock"

local mock = {}

--- Create a call recorder function.
--- @return table: { calls = {}, fn = function(...) }
function mock.recorder()
	local rec = { calls = {} }
	rec.fn = function(...)
		table.insert(rec.calls, { ... })
	end
	return rec
end

--- Install mock __hypr_* globals and return recorders keyed by name.
--- @return table: recorders keyed by global name
function mock.install_hypr_globals()
	local recorders = {}

	local names = {
		"__hypr_add_monitor",
		"__hypr_disable_monitor",
		"__hypr_add_bind",
	}

	for _, name in ipairs(names) do
		local rec = mock.recorder()
		recorders[name] = rec
		_G[name] = rec.fn
	end

	return recorders
end

--- Remove all __hypr_* globals.
function mock.clear_hypr_globals()
	for k, _ in pairs(_G) do
		if type(k) == "string" and k:match("^__hypr_") then
			_G[k] = nil
		end
	end
end

return mock
