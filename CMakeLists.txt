cmake_minimum_required(VERSION 3.16)
project(hyprlua LANGUAGES CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Required system includes (e.g. libdrm)
include_directories(/usr/include/libdrm)

# Find PkgConfig and Pixman
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIXMAN REQUIRED pixman-1)
pkg_check_modules(HYPRLAND REQUIRED hyprland)

# Find Threads
find_package(Threads REQUIRED)

# Find Lua >= 5.3 using pkg-config
pkg_check_modules(LUA REQUIRED lua>=5.3)

# Add the plugin as a shared library
add_library(hyprlua SHARED
  src/main.cpp
  src/globals.cpp
  src/watcher.cpp
  src/utils.cpp
  src/lua/runtime.cpp
  src/lua/monitors.cpp
)

# Include directories
target_include_directories(hyprlua PRIVATE
  src/
  ${PIXMAN_INCLUDE_DIRS}
  ${LUA_INCLUDE_DIRS}
  ${HYPRLAND_INCLUDE_DIRS}
)

# Link against required libraries
target_link_libraries(hyprlua PRIVATE
  ${PIXMAN_LIBRARIES}
  ${LUA_LIBRARIES}
  ${HYPRLAND_LIBRARIES}
  Threads::Threads
)

# Output directory for the compiled plugin
set_target_properties(hyprlua PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)
