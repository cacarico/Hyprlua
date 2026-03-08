# Changelog

All notable changes to Hyprlua will be documented in this file.

## [0.0.1] - 2026-03-08

### Added
- Keybinds module (`hypr.binds.set()`, `hypr.binds.submap()`) with flag and submap support
- Automatic catchall reset for submaps
- Keybind persistence across hot-reload (track and clear plugin-added binds)
- Lua unit test suite using vendored luaunit v3.4
- Mock helpers for `__hypr_*` C++ globals and fresh-require test isolation
- `make test` target for running the test suite
- LDoc annotations for all modules

### Changed
- Migrated Lua runtime from LuaJIT to Lua 5.4
- Rewrote README with updated build/install/usage instructions
- Removed Hyprland git submodule from extern/ (uses system headers via pkg-config)
- Simplified CMakeLists.txt dependency resolution

### Fixed
- File watcher path joining for config directory detection
- `os` stdlib now opened in Lua runtime
- `utils.validate` crash when `expected_types` is a single string instead of a table
- Hot-reload now properly clears hooks and keybinds before reinitializing
