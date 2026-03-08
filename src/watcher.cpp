#include "watcher.hpp"
#include "utils.hpp"
#include "lua/runtime.hpp"
#include "logger.hpp"
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <chrono>

FileWatcher::FileWatcher(const std::string& filepath, const std::string& directory) : m_filepath(filepath), m_directory(directory), m_keepWatching(true) {}

FileWatcher::~FileWatcher() {
    stop();
}

void FileWatcher::start() {
    m_thread = std::thread(&FileWatcher::watch, this);
}

void FileWatcher::stop() {
    m_keepWatching = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

/**
 * @brief inotify event loop with 500ms debounce
 *
 * Uses IN_NONBLOCK so the loop can check m_keepWatching between reads
 * without blocking indefinitely. Events within the debounce window are
 * coalesced to avoid redundant reloads from editors that write in
 * multiple steps (write + rename, etc.).
 */
void FileWatcher::watch() {
    int inotifyFd = inotify_init1(IN_NONBLOCK);
    if (inotifyFd < 0) {
        sendNotification("[Hyprlua] inotify_init1 error: " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        return;
    }

    int watchDesc = inotify_add_watch(inotifyFd, m_directory.c_str(), IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE);
    if (watchDesc < 0) {
        sendNotification("[Hyprlua] Failed to add watch for " + m_directory + ": " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        close(inotifyFd);
        return;
    }

    sendNotification("[Hyprlua] Monitoring '" + m_filepath + "' for changes...", CHyprColor{0.2, 1.0, 0.2, 1.0}, 3000);

    auto lastEventTime = std::chrono::steady_clock::now() - std::chrono::seconds(1);

    while (m_keepWatching) {
        char    buffer[4096] __attribute__((aligned(__alignof__(inotify_event))));
        ssize_t numRead = read(inotifyFd, buffer, sizeof(buffer));

        if (numRead > 0) {
            for (char* ptr = buffer; ptr < buffer + numRead;) {
                auto* event = reinterpret_cast<inotify_event*>(ptr);

                if (event->len > 0) {
                    std::string eventFile = m_directory + "/" + event->name;

                    if (eventFile == m_filepath) {
                        auto now = std::chrono::steady_clock::now();
                        if (now - lastEventTime > std::chrono::milliseconds(500)) {
                            hyprlua::log::info("watcher: config changed, reloading Lua runtime");
                            sendNotification("Reloading Lua config...", CHyprColor{0.2, 0.6, 1.0, 1.0}, 2000);
                            hyprlua::reload_lua_runtime();
                            lastEventTime = now;
                        }
                    }
                }

                ptr += sizeof(inotify_event) + event->len;
            }
        } else if (numRead == -1 && errno != EAGAIN) {
            sendNotification("[Hyprlua] read error: " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    inotify_rm_watch(inotifyFd, watchDesc);
    close(inotifyFd);
}
