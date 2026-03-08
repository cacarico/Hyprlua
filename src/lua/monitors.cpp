#include "monitors.hpp"
#include "utils.hpp"
#include "logger.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/helpers/Color.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <sol/sol.hpp>
#include <iostream>
#include <vector>
#include <sstream>

namespace hyprlua::modules {

    /**
     * @brief Apply a monitor rule to a Hyprland output
     * @param name Output name (e.g. "DP-1", "HDMI-A-1")
     * @param resolution "WxH" format (e.g. "1920x1080")
     * @param position "XxY" format (e.g. "0x0")
     * @param scale Display scale factor
     * @param workspaces Optional workspace IDs to assign
     */
    void add_monitor(const std::string& name, const std::string& resolution, const std::string& position, const double_t& scale, sol::optional<std::vector<double_t>> workspaces) {
        sendNotification("Applying monitor with Hyprland API", CHyprColor{1.0, 0.5, 0.0, 1.0}, 5000);
        log::info("Starting add_monitor");

        std::ostringstream debugLog;
        debugLog << "Add Monitor:\n"
                 << "  Name:       " << name << "\n"
                 << "  Resolution: " << resolution << "\n"
                 << "  Position:   " << position << "\n"
                 << "  Scale:      " << scale << "\n";

        if (workspaces) {
            debugLog << "  Workspaces: ";
            for (const auto& w : workspaces.value())
                debugLog << w << " ";
            debugLog << "\n";
        }
        log::debug(debugLog.str());

        SMonitorRule rule;
        rule.name = name;

        int resX = 1920, resY = 1080;
        sscanf(resolution.c_str(), "%dx%d", &resX, &resY);
        rule.resolution = Vector2D(resX, resY);

        int posX = 0, posY = 0;
        sscanf(position.c_str(), "%dx%d", &posX, &posY);
        rule.offset = Vector2D(posX, posY);

        rule.scale    = static_cast<float>(scale);
        rule.disabled = false;

        auto monitor = g_pCompositor->getMonitorFromName(name);
        if (!monitor) {
            log::error("Monitor not found: " + name);
            std::cerr << "[hyprlua] Monitor not found: " << name << "\n";
            return;
        }

        monitor->applyMonitorRule(&rule, true);
        log::info("Monitor rule applied: " + name);
        sendNotification("Monitor rule applied", CHyprColor{0.0, 1.0, 0.0, 1.0}, 3000);
    }

    void disable_monitor(const std::string& name) {
        sendNotification("Disabling monitor", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        log::info("Disabling monitor: " + name);

        SMonitorRule rule;
        rule.name     = name;
        rule.disabled = true;

        auto monitor = g_pCompositor->getMonitorFromName(name);

        if (!monitor) {
            log::error("Monitor not found for disabling: " + name);
            std::cerr << "[hyprlua] Monitor not found: " << name << "\n";
            return;
        }

        monitor->applyMonitorRule(&rule, true);
        log::info("Monitor disabled: " + name);
        sendNotification("Monitor disabled", CHyprColor{1.0, 0.0, 0.0, 1.0}, 3000);
    }

    std::vector<std::string> list_monitors() {
        std::vector<std::string> names;
        for (auto& pMon : g_pCompositor->m_realMonitors) {
            if (!pMon)
                continue;
            names.emplace_back(pMon->m_name);
        }
        return names;
    }

    void bind_monitors(sol::state& lua) {
        sendNotification("Binding monitors module", CHyprColor{0.2, 0.8, 0.2, 1.0}, 4000);
        log::info("Binding monitor Lua functions");

        lua.set_function("__hypr_add_monitor", &add_monitor);
        lua.set_function("__hypr_disable_monitor", &disable_monitor);

        std::cout << "[hyprlua] Monitors module loaded.\n";
        log::debug("Monitors module successfully bound.");
    }

} // namespace hyprlua::modules
