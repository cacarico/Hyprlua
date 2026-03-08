PREFIX ?= /usr
BUILD_TYPE ?= Release
BUILD_DIR = build
JOBS = $(shell nproc)

all: build ## Build the plugin

clean: ## Clear build files
	@rm -rf $(BUILD_DIR) .cache

build: ## Build Hyprlua
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	cmake --build $(BUILD_DIR) -j$(JOBS)

install: build ## Install system-wide (requires sudo)
	sudo cmake --install $(BUILD_DIR)

uninstall: ## Remove installed files (requires sudo)
	sudo rm -f $(PREFIX)/lib/hyprland/plugins/libhyprlua.so
	sudo rm -rf $(PREFIX)/share/hyprlua

load: ## Load installed plugin into Hyprland
	@hyprctl plugin list | grep -q 'Plugin Hyprlua' && hyprctl plugin unload $(PREFIX)/lib/hyprland/plugins/libhyprlua.so || true
	hyprctl plugin load $(PREFIX)/lib/hyprland/plugins/libhyprlua.so

unload: ## Unload installed plugin from Hyprland
	hyprctl plugin unload $(PREFIX)/lib/hyprland/plugins/libhyprlua.so

reload: unload load ## Reload installed plugin (unload + load)

dev-load: build ## Build and load plugin from build dir
	@hyprctl plugin list | grep -q 'Plugin Hyprlua' && hyprctl plugin unload $(CURDIR)/build/libhyprlua.so || true
	HYPRLUA_MODULES_PATH=$(CURDIR)/runtime/modules hyprctl plugin load $(CURDIR)/build/libhyprlua.so

dev-unload: ## Unload local plugin from Hyprland
	hyprctl plugin unload $(CURDIR)/build/libhyprlua.so

dev-reload: dev-unload dev-load ## Rebuild and reload local plugin

lint: lint-lua ## Lint all files

lint-lua: ## Format and lint Lua files
	@find . -name "*.lua" -type f -exec stylua {} \;

help: ## This help.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.PHONY: all build clean install uninstall load unload reload dev-load dev-unload dev-reload lint lint-lua help
.DEFAULT_GOAL := help
