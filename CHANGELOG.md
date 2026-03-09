# Changelog

## [0.0.2] - 2026-03-09 (Hyprland 0.54.1)

- Fix mouse drag keybinds (flag `m`) crashing Hyprland — handler must be `"mouse"` with action as arg, mirroring native `bindm` parser
- Fix file watcher calling `reload_lua_runtime()` from background thread — dispatch via `g_pEventLoopManager->doLater()` instead
- Fix `os` stdlib not available in user config (`sol::lib::os` was missing from `open_libraries`)
- Fix file watcher path joining for config change detection
- Fix `initialized` flag set before user config runs so hot-reload retries on config errors
- Fix `clean` target not removing generated docs
- Fix `utils.validate` crash when `expected_types` is a string instead of a table
- Add crash signal handler (SIGSEGV, SIGABRT, SIGBUS) — writes backtrace to log before re-raising
- Add keybinds module (`hypr.binds.set`, `hypr.binds.submap`) with flag and submap support
- Add monitors module (`hypr.monitors.add`)
- Add Lua sandbox — block `os.execute`, `os.remove`, `os.rename`, `os.exit`, `package.loadlib`, clear `package.cpath`
- Add per-module error handling with `safe_script_file()` — broken module no longer kills the runtime
- Add Lua unit test suite (`make test`) using vendored luaunit
- Add LDoc annotations and `make docs-lua`
- Rewrite FileWatcher with `poll()` + `eventfd` — `stop()` unblocks instantly, RAII `UniqueFd` for all fds
- Move log file from `/tmp/hyprlua.log` to `$XDG_RUNTIME_DIR/hyprlua.log`
- Migrate Lua runtime from LuaJIT to Lua 5.4

## [0.0.1] - 2026-03-08

- Initial release
