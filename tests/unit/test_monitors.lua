--- Tests for runtime.modules.monitors
--- @module "test_monitors"

local lu = require("lib.luaunit")
local setup = require("helpers.setup")

TestMonitorsAdd = {}

function TestMonitorsAdd:setUp()
	self.monitors, self.recorders = setup.fresh_require("runtime.modules.monitors")
end

function TestMonitorsAdd:test_add_with_all_params()
	self.monitors.add("DP-1", "1920x1080@60Hz", "0x0", 1.0, { 1, 2, 3 })
	local calls = self.recorders.__hypr_add_monitor.calls
	lu.assertEquals(#calls, 1)
	lu.assertEquals(calls[1][1], "DP-1")
	lu.assertEquals(calls[1][2], "1920x1080@60Hz")
	lu.assertEquals(calls[1][3], "0x0")
	lu.assertEquals(calls[1][4], 1.0)
	lu.assertEquals(calls[1][5], { 1, 2, 3 })
end

function TestMonitorsAdd:test_add_with_nil_workspaces()
	self.monitors.add("DP-1", "1920x1080@60Hz", "0x0", 1.0, nil)
	local calls = self.recorders.__hypr_add_monitor.calls
	lu.assertEquals(#calls, 1)
	-- workspaces is passed as nil to C++
	lu.assertNil(calls[1][5])
end

function TestMonitorsAdd:test_wrong_type_name()
	lu.assertErrorMsgContains("name must be a string", function()
		self.monitors.add(123, "1920x1080@60Hz", "0x0", 1.0)
	end)
end

function TestMonitorsAdd:test_wrong_type_resolution()
	lu.assertErrorMsgContains("Resolution must be a string", function()
		self.monitors.add("DP-1", 1920, "0x0", 1.0)
	end)
end

function TestMonitorsAdd:test_wrong_type_position()
	lu.assertErrorMsgContains("Position must be a string", function()
		self.monitors.add("DP-1", "1920x1080@60Hz", 0, 1.0)
	end)
end

function TestMonitorsAdd:test_wrong_type_scale()
	lu.assertErrorMsgContains("Scale must be a number", function()
		self.monitors.add("DP-1", "1920x1080@60Hz", "0x0", "1.0")
	end)
end

function TestMonitorsAdd:test_wrong_type_workspaces()
	lu.assertErrorMsgContains("Workspaces must be a table", function()
		self.monitors.add("DP-1", "1920x1080@60Hz", "0x0", 1.0, "not a table")
	end)
end

function TestMonitorsAdd:test_missing_binding_errors()
	_G.__hypr_add_monitor = nil
	lu.assertErrorMsgContains("__hypr_add_monitor is not defined", function()
		self.monitors.add("DP-1", "1920x1080@60Hz", "0x0", 1.0)
	end)
end

TestMonitorsDisable = {}

function TestMonitorsDisable:setUp()
	self.monitors, self.recorders = setup.fresh_require("runtime.modules.monitors")
end

function TestMonitorsDisable:test_disable_valid()
	self.monitors.disable("eDP-1")
	local calls = self.recorders.__hypr_disable_monitor.calls
	lu.assertEquals(#calls, 1)
	lu.assertEquals(calls[1][1], "eDP-1")
end

function TestMonitorsDisable:test_disable_wrong_type()
	lu.assertErrorMsgContains("name must be a string", function()
		self.monitors.disable(42)
	end)
end

function TestMonitorsDisable:test_missing_disable_binding()
	_G.__hypr_disable_monitor = nil
	lu.assertErrorMsgContains("__hypr_disable_monitor is not defined", function()
		self.monitors.disable("eDP-1")
	end)
end

TestMonitorsAttachment = {}

function TestMonitorsAttachment:setUp()
	self.monitors, self.recorders = setup.fresh_require("runtime.modules.monitors")
end

function TestMonitorsAttachment:test_module_attached_to_hypr()
	lu.assertNotNil(hypr.monitors)
	lu.assertEquals(hypr.monitors, self.monitors)
end
