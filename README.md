# Hyprlua

A Hyprland plugin that embeds a Lua 5.4 runtime, allowing you to configure Hyprland via `~/.config/hypr/hyprland.lua` instead of the native config format.

## Usage

Create a `hyprland.lua` file in your Hyprland config directory (usually `~/.config/hypr/`):

```lua
local bind = hypr.binds.set
local submap = hypr.binds.submap

-- Monitors
hypr.monitors.add("DP-2", "1920x1200", "0x0", 1, { 1, 2 })
hypr.monitors.add("HDMI-A-1", "preferred", "1920x0", 1, { 3, 4, 5 })
hypr.monitors.add("eDP-1", "preferred", "auto", 1, { 6, 7 })

-- Keybinds
bind("SUPER", "Return", "exec", "alacritty")
bind("SUPER", "w", "killactive", "")
bind("SUPER SHIFT", "h", "resizeactive", "-50 0", { flags = "e" })

-- Submaps
bind("SUPER", "o", "submap", "open")
submap("open", function(b)
    b("SUPER", "f", "exec", "nautilus")
    b("SUPER", "s", "exec", "spotify-launcher")
end)
```

The plugin watches your config file and hot-reloads on save.

## Available Features

### Monitors (`hypr.monitors`)

| Function | Description |
|---|---|
| `hypr.monitors.add(name, resolution, position, scale, workspaces?)` | Configure a monitor |
| `hypr.monitors.disable(name)` | Disable a monitor |

- `name` — Monitor identifier (e.g. `"DP-2"`, `"desc:LG Electronics 0x1234"`)
- `resolution` — `"WxH@Hz"`, `"WxH"`, or `"preferred"`
- `position` — `"XxY"` or `"auto"`
- `scale` — Scaling factor (e.g. `1`, `1.5`)
- `workspaces` — Optional list of workspace IDs to assign (e.g. `{ 1, 2, 3 }`)

```lua
hypr.monitors.add("DP-2", "1920x1200@60.00Hz", "0x0", 1, { 1, 2 })
hypr.monitors.add("eDP-1", "preferred", "auto", 1.5)
hypr.monitors.disable("HDMI-A-1")
```

### Keybinds (`hypr.binds`)

| Function | Description |
|---|---|
| `hypr.binds.set(mods, key, dispatcher, args, opts?)` | Bind a key combination |
| `hypr.binds.submap(name, fn)` | Define a submap with grouped binds |

- `mods` — Modifier keys (e.g. `"SUPER"`, `"SUPER SHIFT"`)
- `key` — Key name (e.g. `"Return"`, `"h"`, `"catchall"`)
- `dispatcher` — Hyprland dispatcher (e.g. `"exec"`, `"movefocus"`, `"killactive"`)
- `args` — Dispatcher arguments
- `opts` — Optional table:
  - `flags` — String of flag characters: `l` (locked), `r` (release), `e` (repeat), `m` (mouse), `n` (non-consuming), `t` (transparent), `i` (ignore mods)
  - `submap` — Register the bind in a named submap

```lua
local bind = hypr.binds.set

bind("SUPER", "Return", "exec", "alacritty")
bind("SUPER", "w", "killactive", "")
bind("SUPER SHIFT", "h", "resizeactive", "-50 0", { flags = "e" })

-- Mouse binds
bind("SUPER", "mouse:272", "movewindow", "", { flags = "m" })

-- Submaps (auto-appends catchall reset bind)
hypr.binds.submap("open", function(b)
    b("SUPER", "f", "exec", "nautilus")
    b("SUPER", "s", "exec", "spotify-launcher")
end)
```

### Exec (`hypr.exec`)

| Function | Description |
|---|---|
| `hypr.exec(cmd)` | Spawn a process every time the config is evaluated |
| `hypr.exec_once(cmd)` | Spawn a process only once per Hyprland session |
| `hypr.exec_on_load(cmd)` | Spawn a process once per plugin load (not on hot-reloads) |

- `hypr.exec(cmd)` — Runs on every config load/reload. Use for commands that are safe to repeat.
- `hypr.exec_once(cmd)` — Runs only once per Hyprland session, even across plugin reloads. Tracked via a flag file in `$XDG_RUNTIME_DIR`.
- `hypr.exec_on_load(cmd)` — Runs once when the plugin is loaded via `hyprctl plugin load`, but not on config hot-reloads.

```lua
-- Start a wallpaper daemon once per session
hypr.exec_once("hyprpaper")

-- Launch a bar on plugin load (restarts if plugin is reloaded)
hypr.exec_on_load("waybar")

-- Run on every config evaluation
hypr.exec("notify-send 'Config loaded'")
```

## Installation

### AUR (Arch Linux)

```bash
yay -S hyprlua-git
```

Then add to your `hyprland.conf`:

```ini
plugin = /usr/lib/hyprland/plugins/libhyprlua.so
```

### hyprpm

```bash
hyprpm add https://github.com/cacarico/hyprlua
hyprpm enable hyprlua
```

### From source

```bash
make build      # outputs build/libhyprlua.so
make install    # install system-wide (requires sudo)
make uninstall  # remove installed files
```

Then add to your `hyprland.conf`:

```ini
plugin = /usr/lib/hyprland/plugins/libhyprlua.so
```

## Loading the plugin (source installs)

```bash
make load       # load installed plugin into Hyprland
make unload     # unload from Hyprland
make reload     # unload + load
```

## Development

Build and load the plugin directly from the build directory without installing:

```bash
make dev-load     # build and load from ./build/
make dev-unload   # unload local plugin
make dev-reload   # rebuild and reload
```

### Environment variables

| Variable | Default | Purpose |
|---|---|---|
| `HYPRLUA_CONFIG_PATH` | `~/.config/hypr/hyprland.lua` | User config location |
| `HYPRLUA_MODULES_PATH` | `/usr/share/hyprlua/modules` | Lua module directory |

### Watch logs

```bash
tail -f /tmp/hyprlua.log
```

### Run tests

```bash
make test
```

### Lint

```bash
make lint
```

### All available commands

```bash
make help
```
