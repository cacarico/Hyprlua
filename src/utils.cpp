#include "utils.hpp"
#include "globals.hpp"
#include <cstdlib>
#include <string>

std::mutex notificationMutex;

void       sendNotification(const std::string& message, const CHyprColor& color, int duration) {
    std::lock_guard<std::mutex> lock(notificationMutex);
    if (PHANDLE) {
        HyprlandAPI::addNotification(PHANDLE, message, color, duration);
    }
}

std::string expandTilde(const std::string& path) {
    if (path.empty()) {
        return "";
    }

    if (path[0] == '~') {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + path.substr(1);
        }
        return path.substr(1);
    }

    return path;
}
