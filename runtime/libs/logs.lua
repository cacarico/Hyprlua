--- Logs Module
--- This module handles logs and colors
--- @module "logs"

local logs = {}
--
-- ANSI color codes for enhanced terminal output
local colors = {
	reset = "\27[0m",
	red = "\27[31m",
	green = "\27[32m",
	yellow = "\27[33m",
	blue = "\27[34m",
	magenta = "\27[35m",
	cyan = "\27[36m",
	white = "\27[37m",
}

--- Prints colored message
local function print_colored(color, message)
	print(color .. message .. colors.reset)
end

--- Logs error message
---@param message any
function logs.error(message)
	print_colored(colors.red, message)
end

--- Logs a message
---@param message any
function logs.print(message)
	print_colored(colors.white, message)
end

return logs
