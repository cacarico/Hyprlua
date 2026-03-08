--- Logs Module
--- ANSI-colored terminal output helpers for Lua-side logging.
--- @module "logs"

local logs = {}

--- ANSI color codes for terminal output.
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

--- Print a message with the given ANSI color.
--- @param color string: ANSI escape sequence
--- @param message string: Text to print
local function print_colored(color, message)
	print(color .. message .. colors.reset)
end

--- Log an error message in red.
--- @param message string: Error text
function logs.error(message)
	print_colored(colors.red, message)
end

--- Log a general message in white.
--- @param message string: Message text
function logs.print(message)
	print_colored(colors.white, message)
end

return logs
