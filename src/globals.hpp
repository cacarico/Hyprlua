/**
 * @file globals.hpp
 * @brief Plugin-wide global state
 */
#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <memory>

inline HANDLE PHANDLE = nullptr;

class FileWatcher;
extern std::unique_ptr<FileWatcher> g_FileWatcher;
