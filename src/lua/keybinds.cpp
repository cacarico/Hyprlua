// keybinds.cpp
#include "keybinds.hpp"
#include "logger.hpp"

#include <hyprland/src/managers/KeybindManager.hpp>
#include <sol/sol.hpp>
#include <iostream>
#include <vector>

namespace hyprlua::modules {

    struct PluginBind {
        uint32_t   modmask;
        SParsedKey parsedKey;
    };

    static std::vector<PluginBind> g_pluginBinds;

    void add_bind(const std::string& mods, const std::string& key, const std::string& dispatcher, const std::string& args, const std::string& flags_str) {
        log::info("add_bind: ENTER mods=" + mods + " key=" + key + " dispatcher=" + dispatcher + " args=" + args + " flags=" + flags_str);

        if (!g_pKeybindManager) {
            log::error("add_bind: g_pKeybindManager is null!");
            return;
        }

        SKeybind kb;
        kb.submap  = g_pKeybindManager->getCurrentSubmap();
        kb.key     = key;
        kb.modmask = g_pKeybindManager->stringToModMask(mods);
        kb.handler = dispatcher;
        kb.arg     = args;

        log::debug("add_bind: submap='" + kb.submap.name + "' modmask=" + std::to_string(kb.modmask));

        for (char c : flags_str) {
            switch (c) {
                case 'l': kb.locked       = true; break;
                case 'r': kb.release      = true; break;
                case 'e': kb.repeat       = true; break;
                case 'm': kb.mouse        = true; break;
                case 'n': kb.nonConsuming = true; break;
                case 't': kb.transparent  = true; break;
                case 'i': kb.ignoreMods   = true; break;
                default: log::debug("add_bind: unknown flag '" + std::string(1, c) + "'"); break;
            }
        }

        size_t before = g_pKeybindManager->m_keybinds.size();
        g_pKeybindManager->addKeybind(kb);
        g_pluginBinds.push_back({kb.modmask, {kb.key, kb.keycode, kb.catchAll}});
        size_t after = g_pKeybindManager->m_keybinds.size();
        log::info("add_bind: keybinds count before=" + std::to_string(before) + " after=" + std::to_string(after));

        g_pKeybindManager->shadowKeybinds();
        log::info("add_bind: done, registered " + mods + " + " + key + " -> " + dispatcher);
    }

    void clear_plugin_binds() {
        log::info("clear_plugin_binds: removing " + std::to_string(g_pluginBinds.size()) + " binds");
        for (auto& b : g_pluginBinds)
            g_pKeybindManager->removeKeybind(b.modmask, b.parsedKey);
        g_pluginBinds.clear();
    }

    void bind_keybinds(sol::state& lua) {
        log::info("Binding keybinds Lua functions");
        lua.set_function("__hypr_add_bind", &add_bind);
        std::cout << "[hyprlua] Keybinds module loaded.\n";
        log::debug("Keybinds module successfully bound.");
    }

} // namespace hyprlua::modules
