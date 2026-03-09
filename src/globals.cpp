#include "globals.hpp"
#include "watcher.hpp"
#include <hyprland/src/plugins/PluginAPI.hpp>

HANDLE                       PHANDLE      = nullptr;
std::unique_ptr<FileWatcher> g_FileWatcher = nullptr;
