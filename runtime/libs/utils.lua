--- Utils Module
--- Shared utility functions for parameter validation, table merging, and serialization.
--- @module "utils"

local utils = {}

--- Validate parameter types.
--- Each entry in `params` is `{ value, expected_type(s), optional }`.
--- @param params table: Map of param_name → {value, type_or_types, optional?}
function utils.validate(params)
	for param_name, param_info in pairs(params) do
		local value, expected_types, optional = param_info[1], param_info[2], param_info[3]

		if value == nil then
			if not optional then
				error(string.format("Parameter '%s' is required.", param_name))
			end
		else
			local value_type = type(value)
			local valid_type = false

			for _, expected_type in ipairs(type(expected_types) == "table" and expected_types or { expected_types }) do
				if value_type == expected_type then
					valid_type = true
					break
				end
			end
			if not valid_type then
				error(
					string.format(
						"Parameter '%s' must be of type %s.",
						param_name,
						table.concat(expected_types, " or ")
					)
				)
			end
		end
	end
end

--- Deep-merge `opts` into `defaults`, returning the merged table.
--- Nested tables are merged recursively; scalar values are overwritten.
--- @param defaults table: Base table with default values
--- @param opts table: User-supplied overrides
function utils.merge_tables(defaults, opts)
	if type(opts) ~= "table" then
		return defaults
	end
	for k, v in pairs(opts) do
		if type(v) == "table" and type(defaults[k] or nil) == "table" then
			defaults[k] = utils.merge_tables(defaults[k], v)
		else
			defaults[k] = v
		end
	end
	return defaults
end

--- Serialize a configuration table into a Hyprland-style formatted string.
--- Handles nested tables and flattens `col.*` sub-keys.
--- @param config table: The configuration table to serialize
--- @param config_type string: Section header name (e.g. "general", "decoration")
--- @return string: The serialized configuration block
function utils.serialize_config(config, config_type)
	local serialized_lines = {}
	local indent_char = "    " -- 4 spaces for indentation

	-- Helper function to add a line with proper indentation
	local function add_line(line, indent_level)
		local indent = indent_char:rep(indent_level)
		table.insert(serialized_lines, indent .. line)
	end

	-- Start with the header
	add_line(string.format("%s {", config_type), 0)

	-- Recursive function to serialize nested tables
	local function serialize_table(current_table, indent_level)
		-- Collect and sort keys for consistent ordering
		local keys = {}
		for key in pairs(current_table) do
			table.insert(keys, key)
		end
		table.sort(keys)

		for _, key in ipairs(keys) do
			local value = current_table[key]

			if key == "col" and type(value) == "table" then
				-- Flatten the 'col' table by prefixing keys with 'col.'
				local subkeys = {}
				for subkey in pairs(value) do
					table.insert(subkeys, subkey)
				end
				table.sort(subkeys)

				for _, subkey in ipairs(subkeys) do
					local subvalue = value[subkey]
					local full_key = "col." .. subkey

					if type(subvalue) == "table" then
						-- Handle nested tables within 'col' if any
						add_line(string.format("%s = {", full_key), indent_level)
						serialize_table(subvalue, indent_level + 1)
						add_line("}", indent_level)
					else
						-- Serialize key-value pair
						local serialized_value
						if type(subvalue) == "string" then
							serialized_value = string.format('"%s"', subvalue)
						elseif type(subvalue) == "number" or type(subvalue) == "boolean" then
							serialized_value = tostring(subvalue)
						else
							serialized_value = "nil"
						end
						add_line(string.format("%s = %s", full_key, serialized_value), indent_level)
					end
				end
			else
				if type(value) == "table" then
					-- For other tables, create a nested table
					add_line(string.format("%s = {", key), indent_level)
					serialize_table(value, indent_level + 1)
					add_line("}", indent_level)
				else
					-- Serialize key-value pair
					local serialized_value
					if type(value) == "string" then
						serialized_value = string.format('"%s"', value)
					elseif type(value) == "number" or type(value) == "boolean" then
						serialized_value = tostring(value)
					else
						serialized_value = "nil"
					end
					add_line(string.format("%s = %s", key, serialized_value), indent_level)
				end
			end
		end
	end

	-- Start serialization with initial indent level of 1
	serialize_table(config, 1)

	-- Close the main table
	add_line("}", 0)

	-- Combine all lines into a single string
	return table.concat(serialized_lines, "\n")
end

return utils
