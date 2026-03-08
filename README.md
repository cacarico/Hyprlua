# Hyprlua

Hyprlua aims to end my frustration with configuring Hyprland using its configuration file.
It create a Lua Runtime that reads the config file and issue commands to Hyprland.


## Using

Create a hyprland.lua file on Hyprland directory (usually `~/.config/hypr`)

```lua

-- Create binds
hyprlua.binds.set("SUPER SHIFT", "h", "resizeactive", "-50 0")
hyprlua.binds.set("SUPER SHIFT", "j", "resizeactive", "0 50")
hyprlua.binds.set("SUPER SHIFT", "k", "resizeactive", "0 -50")
hyprlua.binds.set("SUPER SHIFT", "l", "resizeactive", "50 0")

hyprlua.monitors.add("DP-2", "1920x1200", "0x0", 1, { 1, 2 })
hyprlua.monitors.add("HDMI-A-1", "preferred", "1920x0", 1, { 3, 4, 5 })
hyprlua.monitors.add("eDP-1", "preferred", "auto", 1, { 6, 7 })
```

## Development

### Building locally

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
# Output: build/libhyprlua.so
```

### Local dev without installing

Set env vars so Hyprland uses your local module files instead of the installed ones:

```bash
export HYPRLUA_MODULES_PATH=/path/to/hyprlua/runtime/modules
export HYPRLUA_CONFIG_PATH=~/.config/hypr/hyprland.lua
```

### Load / reload the plugin

```bash
# Load
hyprctl plugin load /path/to/hyprlua/build/libhyprlua.so

# Reload (unload then load to pick up C++ changes)
hyprctl plugin unload /path/to/hyprlua/build/libhyprlua.so
hyprctl plugin load  /path/to/hyprlua/build/libhyprlua.so
```

### Watch logs

```bash
tail -f /tmp/hyprlua.log
```

### Install system-wide

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --install build
```
