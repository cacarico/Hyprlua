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
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
# Output: build/libhyprlua.so
```

### Load the plugin

```bash
hyprctl plugin load /path/to/hyprlua/build/libhyprlua.so
```

### Install system-wide

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --install build
```

## Development

### Local dev without installing

Set env vars so Hyprland uses your local module files instead of the installed ones:

```bash
export HYPRLUA_MODULES_PATH=/path/to/hyprlua/runtime/modules
export HYPRLUA_CONFIG_PATH=~/.config/hypr/hyprland.lua
```

### Reload the plugin (after C++ changes)

```bash
hyprctl plugin unload /path/to/hyprlua/build/libhyprlua.so
hyprctl plugin load   /path/to/hyprlua/build/libhyprlua.so
```

### Watch logs

```bash
tail -f /tmp/hyprlua.log
```

### Run tests

```bash
cd tests && lua5.4 run_all_tests.lua
```
