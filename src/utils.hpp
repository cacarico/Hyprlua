/**
 * @file utils.hpp
 * @brief Shared utility functions
 */
#pragma once
#include <hyprland/src/helpers/Color.hpp>
#include <string>

/**
 * @brief Send a notification through Hyprland's notification system
 * @param message Text content to display
 * @param color Notification color (RGBA, 0.0-1.0)
 * @param duration Display duration in milliseconds
 */
void sendNotification(const std::string& message, const CHyprColor& color, int duration);

/// @brief Expand leading ~ to $HOME
std::string expandTilde(const std::string& path);
