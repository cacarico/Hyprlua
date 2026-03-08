--- Tests for runtime.libs.utils
--- @module "test_utils"

local lu = require("lib.luaunit")
local utils = require("runtime.libs.utils")

TestValidate = {}

function TestValidate:test_valid_params_pass()
	utils.validate({
		name = { "hello", "string" },
		count = { 42, "number" },
	})
	-- No error means pass
end

function TestValidate:test_required_missing_throws()
	lu.assertErrorMsgContains("required", function()
		utils.validate({
			name = { nil, "string" },
		})
	end)
end

function TestValidate:test_optional_nil_passes()
	utils.validate({
		name = { nil, "string", true },
	})
end

function TestValidate:test_wrong_type_throws()
	lu.assertErrorMsgContains("must be of type", function()
		utils.validate({
			name = { 123, "string" },
		})
	end)
end

function TestValidate:test_wrong_type_table_spec_throws()
	lu.assertErrorMsgContains("must be of type", function()
		utils.validate({
			name = { 123, { "string" } },
		})
	end)
end

function TestValidate:test_multi_type_accepted()
	utils.validate({
		value = { "hello", { "string", "number" } },
	})
	utils.validate({
		value = { 42, { "string", "number" } },
	})
end

function TestValidate:test_multi_type_wrong_throws()
	lu.assertErrorMsgContains("must be of type", function()
		utils.validate({
			value = { true, { "string", "number" } },
		})
	end)
end

TestMergeTables = {}

function TestMergeTables:test_scalar_override()
	local result = utils.merge_tables({ a = 1 }, { a = 2 })
	lu.assertEquals(result.a, 2)
end

function TestMergeTables:test_deep_nested_merge()
	local result = utils.merge_tables(
		{ outer = { inner = 1, keep = true } },
		{ outer = { inner = 2 } }
	)
	lu.assertEquals(result.outer.inner, 2)
	lu.assertEquals(result.outer.keep, true)
end

function TestMergeTables:test_non_table_opts_returns_defaults()
	local defaults = { a = 1 }
	local result = utils.merge_tables(defaults, "not a table")
	lu.assertEquals(result.a, 1)
end

function TestMergeTables:test_new_keys_added()
	local result = utils.merge_tables({ a = 1 }, { b = 2 })
	lu.assertEquals(result.a, 1)
	lu.assertEquals(result.b, 2)
end

function TestMergeTables:test_nested_table_replaced_by_scalar()
	local result = utils.merge_tables({ a = { nested = true } }, { a = 42 })
	lu.assertEquals(result.a, 42)
end

TestSerializeConfig = {}

function TestSerializeConfig:test_simple_number()
	local result = utils.serialize_config({ gap = 5 }, "general")
	lu.assertStrContains(result, "gap = 5")
	lu.assertStrContains(result, "general {")
end

function TestSerializeConfig:test_quoted_strings()
	local result = utils.serialize_config({ font = "mono" }, "misc")
	lu.assertStrContains(result, 'font = "mono"')
end

function TestSerializeConfig:test_booleans()
	local result = utils.serialize_config({ enabled = true }, "section")
	lu.assertStrContains(result, "enabled = true")
end

function TestSerializeConfig:test_nested_tables()
	local result = utils.serialize_config({ sub = { val = 1 } }, "section")
	lu.assertStrContains(result, "sub = {")
	lu.assertStrContains(result, "val = 1")
end

function TestSerializeConfig:test_col_key_flattening()
	local result = utils.serialize_config({ col = { active = "#ff0000" } }, "general")
	lu.assertStrContains(result, 'col.active = "#ff0000"')
	-- Should NOT contain a standalone "col = {"
	lu.assertNotStrContains(result, "col = {")
end

function TestSerializeConfig:test_string_with_quotes()
	local result = utils.serialize_config({ title = 'say "hello"' }, "misc")
	lu.assertStrContains(result, 'title = "say \\"hello\\""')
end

function TestSerializeConfig:test_string_with_backslash()
	local result = utils.serialize_config({ path = "C:\\Users\\test" }, "misc")
	lu.assertStrContains(result, 'path = "C:\\\\Users\\\\test"')
end

function TestSerializeConfig:test_string_with_newline()
	local result = utils.serialize_config({ text = "line1\nline2" }, "misc")
	lu.assertStrContains(result, 'text = "line1\\nline2"')
end

function TestSerializeConfig:test_sorted_keys()
	local result = utils.serialize_config({ zebra = 1, alpha = 2 }, "section")
	local alpha_pos = result:find("alpha")
	local zebra_pos = result:find("zebra")
	lu.assertTrue(alpha_pos < zebra_pos)
end
