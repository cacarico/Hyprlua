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

/**
 * @brief Launch the monitoring thread
 * @details Creates a new thread that executes the watch() method
 */
void FileWatcher::start() {
    m_thread = std::thread(&FileWatcher::watch, this);
}

/**
 * @brief Stop and join the monitoring thread
 * @details Sets the termination flag and waits for the thread to exit
 */
void FileWatcher::stop() {
    m_keepWatching = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

/**
 * @brief Core file monitoring implementation using inotify
 * @details This method:
 * 1. Initializes inotify in non-blocking mode
 * 2. Sets up watches for modify/close/move/create events
 * 3. Implements a debounced event detection loop
 * 4. Handles errors through UI notifications
 * 5. Maintains a 500ms event processing interval
 *
 * @note Uses IN_NONBLOCK to prevent blocking the thread during exit
 * @note Implements 500ms debouncing to prevent duplicate notifications
 */
void FileWatcher::watch() {
    // Initialize inotify with non-blocking mode
    int inotifyFd = inotify_init1(IN_NONBLOCK);
    if (inotifyFd < 0) {
        sendNotification("[Hyprlua] inotify_init1 error: " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        return;
    }

    // Add a watch on the specified directory for relevant events
    int watchDesc = inotify_add_watch(inotifyFd, m_directory.c_str(), IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE);
    if (watchDesc < 0) {
        sendNotification("[Hyprlua] Failed to add watch for " + m_directory + ": " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        close(inotifyFd);
        return;
    }

    // Notify that monitoring has started
    sendNotification("[Hyprlua] Monitoring '" + m_filepath + "' for changes...", CHyprColor{0.2, 1.0, 0.2, 1.0}, 3000);

    // Initialize the last event time for debouncing
    auto lastEventTime = std::chrono::steady_clock::now() - std::chrono::seconds(1);

    // Event loop
    while (m_keepWatching) {
        char    buffer[4096] __attribute__((aligned(__alignof__(inotify_event))));
        ssize_t numRead = read(inotifyFd, buffer, sizeof(buffer));

        if (numRead > 0) {
            for (char* ptr = buffer; ptr < buffer + numRead;) {
                auto* event = reinterpret_cast<inotify_event*>(ptr);

                if (event->len > 0) {
                    std::string eventFile = m_directory + event->name;

                    if (eventFile == m_filepath) {
                        auto now = std::chrono::steady_clock::now();
                        if (now - lastEventTime > std::chrono::milliseconds(500)) { // Debounce
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
            // An error occurred during read
            sendNotification("[Hyprlua] read error: " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        }

        // Sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Cleanup: Remove the watch and close the inotify file descriptor
    inotify_rm_watch(inotifyFd, watchDesc);
    close(inotifyFd);
}
