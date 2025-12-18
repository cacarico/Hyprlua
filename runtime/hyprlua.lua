--- Hyprlua Library
--- This library handles everything
--- @module "hyprlua"

local monitors = require("runtime.modules.monitors")
local logs = require("runtime.libs.logs")

local hyprlua = {
	monitors = monitors,
}

return hyprlua
