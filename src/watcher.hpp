/**
 * @file watcher.hpp
 * @brief inotify-based file watcher for config hot-reload
 */
#pragma once

#include <string>
#include <thread>
#include <atomic>
#include "globals.hpp"

/**
 * @brief Watches a directory via inotify(7) and triggers Lua runtime
 *        reload when the target config file changes.
 */
class FileWatcher {
  public:
    FileWatcher(const std::string& filepath, const std::string& directory);
    ~FileWatcher();

    void start();
    void stop();

  private:
    void watch();

    std::string       m_filepath;
    std::string       m_directory;
    std::thread       m_thread;
    std::atomic<bool> m_keepWatching;
};
