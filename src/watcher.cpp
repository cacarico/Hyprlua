#include "watcher.hpp"
#include "utils.hpp"
#include "lua/runtime.hpp"
#include "logger.hpp"
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/eventfd.h>
#include <poll.h>
#include <fcntl.h>
#include <chrono>
#include <utility>

/**
 * @brief RAII wrapper for a file descriptor.
 *        Closes the fd on destruction. Not copyable, but moveable.
 */
struct UniqueFd {
    int fd{-1};
    explicit UniqueFd(int fd) noexcept : fd(fd) {}
    ~UniqueFd() noexcept {
        if (fd >= 0)
            ::close(fd);
    }
    UniqueFd(const UniqueFd&)            = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;
    UniqueFd(UniqueFd&& o) noexcept : fd(std::exchange(o.fd, -1)) {}
    operator int() const noexcept { return fd; }
};

FileWatcher::FileWatcher(const std::string& filepath, const std::string& directory)
    : m_filepath(filepath), m_directory(directory), m_keepWatching(true),
      m_wakeupFd(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) {}

FileWatcher::~FileWatcher() {
    stop();
    if (m_wakeupFd >= 0) {
        ::close(m_wakeupFd);
        m_wakeupFd = -1;
    }
}

void FileWatcher::start() {
    m_thread = std::thread(&FileWatcher::watch, this);
}

/**
 * @brief Signal the watcher thread to exit and wait for it to finish.
 *
 *        Writes to m_wakeupFd so poll() unblocks immediately — no 500ms wait.
 *        Safe to call multiple times (idempotent via exchange).
 */
void FileWatcher::stop() {
    if (m_keepWatching.exchange(false) && m_wakeupFd >= 0) {
        const uint64_t val = 1;
        ::write(m_wakeupFd, &val, sizeof(val));
    }
    if (m_thread.joinable())
        m_thread.join();
}

/**
 * @brief inotify event loop with 500ms debounce.
 *
 *        Uses poll() on both the inotify fd and a wakeup eventfd so that
 *        stop() can interrupt the loop immediately without any sleep().
 *        EINTR is handled on every blocking call.
 *        All resources are managed via UniqueFd RAII — no leaks on any exit path.
 */
void FileWatcher::watch() {
    UniqueFd inotifyFd{inotify_init1(IN_NONBLOCK | IN_CLOEXEC)};
    if (inotifyFd < 0) {
        sendNotification("[Hyprlua] inotify_init1 error: " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        return;
    }

    int watchDesc = inotify_add_watch(inotifyFd, m_directory.c_str(), IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE);
    if (watchDesc < 0) {
        sendNotification("[Hyprlua] Failed to add watch for " + m_directory + ": " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        return; // inotifyFd RAII closes it
    }

    auto lastEventTime = std::chrono::steady_clock::now() - std::chrono::seconds(1);

    struct pollfd fds[2];
    fds[0].fd     = inotifyFd;
    fds[0].events = POLLIN;
    fds[1].fd     = m_wakeupFd;
    fds[1].events = POLLIN;

    while (m_keepWatching) {
        int ret;
        do {
            ret = poll(fds, 2, -1);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            sendNotification("[Hyprlua] poll error: " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
            break;
        }

        // Wakeup eventfd fired — stop() was called
        if (fds[1].revents & POLLIN)
            break;

        if (!(fds[0].revents & POLLIN))
            continue;

        alignas(inotify_event) char buffer[4096];
        ssize_t                     numRead;
        do {
            numRead = read(inotifyFd, buffer, sizeof(buffer));
        } while (numRead < 0 && errno == EINTR);

        if (numRead > 0) {
            for (char* ptr = buffer; ptr < buffer + numRead;) {
                auto* event = reinterpret_cast<inotify_event*>(ptr);

                if (event->len > 0) {
                    std::string eventFile = m_directory + "/" + event->name;

                    if (eventFile == m_filepath) {
                        auto now = std::chrono::steady_clock::now();
                        if (now - lastEventTime > std::chrono::milliseconds(500)) {
                            hyprlua::log::info("watcher: config changed, reloading Lua runtime");
                            hyprlua::reload_lua_runtime();
                            lastEventTime = now;
                        }
                    }
                }

                ptr += sizeof(inotify_event) + event->len;
            }
        } else if (numRead < 0 && errno != EAGAIN) {
            sendNotification("[Hyprlua] read error: " + std::string(std::strerror(errno)), CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        }
    }

    inotify_rm_watch(inotifyFd, watchDesc);
    // inotifyFd RAII closes the fd
}
