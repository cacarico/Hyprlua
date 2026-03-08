--- Tests for runtime.libs.logs
--- @module "test_logs"

local lu = require("lib.luaunit")
local logs = require("runtime.libs.logs")

TestLogs = {}

function TestLogs:setUp()
	-- Capture print output
	self.output = {}
	self._orig_print = print
	_G.print = function(...)
		local args = { ... }
		for _, v in ipairs(args) do
			table.insert(self.output, tostring(v))
		end
	end
end

function TestLogs:tearDown()
	_G.print = self._orig_print
end

function TestLogs:test_error_red_ansi()
	logs.error("something broke")
	lu.assertEquals(#self.output, 1)
	lu.assertStrContains(self.output[1], "\27[31m")
	lu.assertStrContains(self.output[1], "something broke")
	lu.assertStrContains(self.output[1], "\27[0m")
end

function TestLogs:test_print_white_ansi()
	logs.print("hello world")
	lu.assertEquals(#self.output, 1)
	lu.assertStrContains(self.output[1], "\27[37m")
	lu.assertStrContains(self.output[1], "hello world")
	lu.assertStrContains(self.output[1], "\27[0m")
end
