--- Test setup helpers.
--- Provides fresh-require to isolate each test from shared state.
--- @module "helpers.setup"

local mock = require("helpers.mock")

local setup = {}

--- Configure package.path so runtime modules resolve from project root.
function setup.configure_paths()
	local project_root = debug.getinfo(1, "S").source:match("@(.*/)tests/") or "../"
	-- no-op: paths are configured by run_all_tests.lua
	package.path = project_root .. "?.lua;"
		.. project_root .. "?/init.lua;"
		.. package.path
end

--- Runtime module names to clear from package.loaded between tests.
local runtime_modules = {
	"runtime.modules.monitors",
	"runtime.modules.binds",
	"runtime.libs.logs",
	"runtime.libs.utils",
	"runtime.hyprlua",
}

--- Require a runtime module with fresh state.
--- Clears package.loaded for all runtime modules, recreates the hypr global,
--- installs mock __hypr_* globals, then requires the requested module.
--- @param module_name string: e.g. "runtime.modules.monitors"
--- @return any, table: the loaded module and the mock recorders
function setup.fresh_require(module_name)
	-- Clear cached modules
	for _, name in ipairs(runtime_modules) do
		package.loaded[name] = nil
	end

	-- Recreate the hypr global (mirrors C++ runtime.cpp behavior)
	_G.hypr = { version = "test" }

	-- Install mocks
	local recorders = mock.install_hypr_globals()

	-- Require the module
	local mod = require(module_name)

	return mod, recorders
end

return setup
