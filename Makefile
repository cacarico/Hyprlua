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

uninstall: ## Uninstall the plugin (uses CMake install manifest when available)
	@if [ -f $(BUILD_DIR)/install_manifest.txt ]; then \
		sudo xargs rm -f < $(BUILD_DIR)/install_manifest.txt; \
	else \
		sudo rm -f $(PREFIX)/lib/hyprland/plugins/libhyprlua.so; \
		sudo rm -rf $(PREFIX)/share/hyprlua; \
	fi

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

test: ## Run Lua unit tests
	cd tests && lua5.4 run_all_tests.lua -v

docs: docs-cpp ## Generate all documentation

docs-cpp: ## Generate C++ docs with Doxygen
	doxygen Doxyfile

format: format-cpp lint-lua ## Format all code (C++ clang-format + Lua stylua)

format-cpp: ## Format C++ source files with clang-format
	@find src/ -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

lint: lint-lua-check ## Run static analysis (luacheck)

lint-lua: ## Format Lua files with stylua
	@find . -name "*.lua" -not -path "./tests/lib/*" -type f -exec stylua {} \;

lint-lua-check: ## Run luacheck static analysis
	luacheck . --no-color

check: lint test ## Run all linting and tests

bump-version: ## Bump version: make bump-version V=X.Y.Z
	@[ -n "$(V)" ] || (echo "Usage: make bump-version V=X.Y.Z"; exit 1)
	@./scripts/bump-version.sh $(V)

help: ## This help.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.PHONY: all build clean install uninstall load unload reload dev-load dev-unload dev-reload test docs docs-cpp format format-cpp lint lint-lua lint-lua-check check bump-version help
.DEFAULT_GOAL := help
