--- Tests for runtime.modules.binds
--- @module "test_binds"

local lu = require("lib.luaunit")
local setup = require("helpers.setup")

TestBindsSet = {}

function TestBindsSet:setUp()
	self.binds, self.recorders = setup.fresh_require("runtime.modules.binds")
end

function TestBindsSet:test_basic_set()
	self.binds.set("SUPER", "Return", "exec", "kitty")
	local calls = self.recorders.__hypr_add_bind.calls
	lu.assertEquals(#calls, 1)
	lu.assertEquals(calls[1][1], "SUPER")
	lu.assertEquals(calls[1][2], "Return")
	lu.assertEquals(calls[1][3], "exec")
	lu.assertEquals(calls[1][4], "kitty")
	lu.assertEquals(calls[1][5], "") -- flags
	lu.assertEquals(calls[1][6], "") -- submap
end

function TestBindsSet:test_set_with_flags()
	self.binds.set("SUPER", "l", "exec", "swaylock", { flags = "l" })
	local calls = self.recorders.__hypr_add_bind.calls
	lu.assertEquals(calls[1][5], "l")
	lu.assertEquals(calls[1][6], "")
end

function TestBindsSet:test_set_with_submap()
	self.binds.set("SUPER", "r", "submap", "resize", { submap = "resize" })
	local calls = self.recorders.__hypr_add_bind.calls
	lu.assertEquals(calls[1][5], "")
	lu.assertEquals(calls[1][6], "resize")
end

function TestBindsSet:test_set_with_both_opts()
	self.binds.set("SUPER", "r", "exec", "cmd", { flags = "e", submap = "mysub" })
	local calls = self.recorders.__hypr_add_bind.calls
	lu.assertEquals(calls[1][5], "e")
	lu.assertEquals(calls[1][6], "mysub")
end

TestBindsSetValidation = {}

function TestBindsSetValidation:setUp()
	self.binds, self.recorders = setup.fresh_require("runtime.modules.binds")
end

function TestBindsSetValidation:test_wrong_type_mods()
	lu.assertErrorMsgContains("mods must be a string", function()
		self.binds.set(123, "q", "exec", "cmd")
	end)
end

function TestBindsSetValidation:test_wrong_type_key()
	lu.assertErrorMsgContains("key must be a string", function()
		self.binds.set("SUPER", 42, "exec", "cmd")
	end)
end

function TestBindsSetValidation:test_wrong_type_dispatcher()
	lu.assertErrorMsgContains("dispatcher must be a string", function()
		self.binds.set("SUPER", "q", 42, "cmd")
	end)
end

function TestBindsSetValidation:test_wrong_type_args()
	lu.assertErrorMsgContains("args must be a string", function()
		self.binds.set("SUPER", "q", "exec", 42)
	end)
end

function TestBindsSetValidation:test_wrong_type_opts()
	lu.assertErrorMsgContains("opts must be a table", function()
		self.binds.set("SUPER", "q", "exec", "cmd", "not a table")
	end)
end

function TestBindsSetValidation:test_wrong_type_opts_flags()
	lu.assertErrorMsgContains("opts.flags must be a string", function()
		self.binds.set("SUPER", "q", "exec", "cmd", { flags = 123 })
	end)
end

function TestBindsSetValidation:test_wrong_type_opts_submap()
	lu.assertErrorMsgContains("opts.submap must be a string", function()
		self.binds.set("SUPER", "q", "exec", "cmd", { submap = 123 })
	end)
end

function TestBindsSetValidation:test_missing_binding()
	_G.__hypr_add_bind = nil
	lu.assertErrorMsgContains("__hypr_add_bind is not defined", function()
		self.binds.set("SUPER", "q", "exec", "cmd")
	end)
end

TestBindsSubmap = {}

function TestBindsSubmap:setUp()
	self.binds, self.recorders = setup.fresh_require("runtime.modules.binds")
end

function TestBindsSubmap:test_callback_receives_bind_helper()
	local received_bind = nil
	self.binds.submap("resize", function(bind)
		received_bind = bind
	end)
	lu.assertNotNil(received_bind)
	lu.assertEquals(type(received_bind), "function")
end

function TestBindsSubmap:test_all_binds_get_correct_submap()
	self.binds.submap("resize", function(bind)
		bind("", "l", "resizeactive", "30 0")
		bind("", "h", "resizeactive", "-30 0")
	end)
	local calls = self.recorders.__hypr_add_bind.calls
	-- 2 user binds + 1 catchall
	lu.assertEquals(#calls, 3)
	lu.assertEquals(calls[1][6], "resize")
	lu.assertEquals(calls[2][6], "resize")
end

function TestBindsSubmap:test_catchall_reset_appended()
	self.binds.submap("resize", function(bind)
		bind("", "l", "resizeactive", "30 0")
	end)
	local calls = self.recorders.__hypr_add_bind.calls
	local last = calls[#calls]
	lu.assertEquals(last[1], "")       -- mods
	lu.assertEquals(last[2], "catchall") -- key
	lu.assertEquals(last[3], "submap")   -- dispatcher
	lu.assertEquals(last[4], "reset")    -- args
	lu.assertEquals(last[6], "resize")   -- submap
end

function TestBindsSubmap:test_submap_forced_even_with_opts()
	self.binds.submap("resize", function(bind)
		bind("SUPER", "l", "resizeactive", "30 0", { submap = "other", flags = "e" })
	end)
	local calls = self.recorders.__hypr_add_bind.calls
	-- submap should be forced to "resize", not "other"
	lu.assertEquals(calls[1][6], "resize")
	-- flags should still be passed through
	lu.assertEquals(calls[1][5], "e")
end

TestBindsAttachment = {}

function TestBindsAttachment:setUp()
	self.binds, self.recorders = setup.fresh_require("runtime.modules.binds")
end

function TestBindsAttachment:test_module_attached_to_hypr()
	lu.assertNotNil(hypr.binds)
	lu.assertEquals(hypr.binds, self.binds)
end
