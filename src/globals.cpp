#include "globals.hpp"
#include "watcher.hpp"
#include <hyprland/src/plugins/PluginAPI.hpp>

std::unique_ptr<FileWatcher> g_FileWatcher = nullptr;
