#!/usr/bin/env lua5.4
--- Test runner for Hyprlua.
--- Configures paths, loads all test suites, and runs luaunit.

-- Resolve project root from this file's location
local script_dir = debug.getinfo(1, "S").source:match("@(.*/)") or "./"
local project_root = script_dir .. "../"

-- Configure package.path: project root (for runtime.*), tests dir (for helpers/lib)
package.path = project_root .. "?.lua;"
	.. project_root .. "?/init.lua;"
	.. script_dir .. "?.lua;"
	.. script_dir .. "?/init.lua;"
	.. package.path

local lu = require("lib.luaunit")

-- Load all test suites
require("unit.test_utils")
require("unit.test_logs")
require("unit.test_monitors")
require("unit.test_binds")

os.exit(lu.LuaUnit.run())
