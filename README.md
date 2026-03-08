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

## Building

```bash
make build
```

Outputs `build/libhyprlua.so`.

## Installation

```bash
make install    # install system-wide (requires sudo)
make uninstall  # remove installed files
```

## Loading the plugin

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
