--- Hyprlua entry point.
--- Assembles the `hypr` global table from submodules.
--- Loaded by the C++ runtime before user config is executed.
--- @module "hyprlua"

local monitors = require("runtime.modules.monitors")
local binds = require("runtime.modules.binds")
local logs = require("runtime.libs.logs")

local hyprlua = {
	monitors = monitors,
	binds = binds,
	logs = logs,
}

return hyprlua
