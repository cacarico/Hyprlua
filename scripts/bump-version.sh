#!/usr/bin/env bash
set -euo pipefail

HYPRLAND_VERSION=$(pkg-config --modversion hyprland 2>/dev/null || echo "unknown")
TODAY=$(date +%Y-%m-%d)

usage() {
    echo "Usage: $0 <new-version>"
    echo "  Example: $0 0.1.0"
    exit 1
}

[[ $# -eq 1 ]] || usage

NEW_VERSION="$1"

# Validate semver format
if ! [[ "$NEW_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Error: version must be in X.Y.Z format (got '$NEW_VERSION')"
    exit 1
fi

# Derive current version from CMakeLists.txt
CURRENT_VERSION=$(grep -oP '(?<=project\(hyprlua VERSION )[0-9]+\.[0-9]+\.[0-9]+' CMakeLists.txt)

if [[ "$NEW_VERSION" == "$CURRENT_VERSION" ]]; then
    echo "Error: new version ($NEW_VERSION) is the same as current ($CURRENT_VERSION)"
    exit 1
fi

echo "Bumping $CURRENT_VERSION → $NEW_VERSION (Hyprland $HYPRLAND_VERSION)"

# Resolve commit pins
PLUGIN_COMMIT=$(git rev-parse HEAD)
HYPRLAND_COMMIT=""
if command -v gh &>/dev/null && [[ "$HYPRLAND_VERSION" != "unknown" ]]; then
    HYPRLAND_COMMIT=$(gh api "repos/hyprwm/Hyprland/commits/v${HYPRLAND_VERSION}" --jq '.sha' 2>/dev/null || true)
fi

# CMakeLists.txt
sed -i "s/project(hyprlua VERSION ${CURRENT_VERSION}/project(hyprlua VERSION ${NEW_VERSION}/" CMakeLists.txt

# hyprpm.toml — add commit pin if hashes resolved
if [[ -n "$HYPRLAND_COMMIT" ]]; then
    sed -i "s|^]$|    [\"${HYPRLAND_COMMIT}\", \"${PLUGIN_COMMIT}\"],\n]|" hyprpm.toml
fi

# CHANGELOG.md — insert new section after the header block (line 4)
CHANGELOG_ENTRY="## [${NEW_VERSION}] - ${TODAY} (Hyprland ${HYPRLAND_VERSION})\n\n### Added\n-\n\n### Changed\n-\n\n### Fixed\n-\n"
sed -i "5s|^|${CHANGELOG_ENTRY}\n|" CHANGELOG.md

echo ""
echo "Updated files:"
echo "  CMakeLists.txt  — version $CURRENT_VERSION → $NEW_VERSION"
if [[ -n "$HYPRLAND_COMMIT" ]]; then
    echo "  hyprpm.toml     — added commit pin (Hyprland $HYPRLAND_VERSION / plugin $(git rev-parse --short HEAD))"
else
    echo "  hyprpm.toml     — skipped commit pin (gh CLI unavailable or Hyprland version unknown)"
fi
echo "  CHANGELOG.md    — new section for $NEW_VERSION"
echo ""
echo "Next steps:"
echo "  1. Fill in CHANGELOG.md entries for $NEW_VERSION"
echo "  2. git add CMakeLists.txt hyprpm.toml CHANGELOG.md"
echo "  3. git commit -m \"chore: bump version to $NEW_VERSION\""
echo "  4. git tag -a v${NEW_VERSION} -m \"v${NEW_VERSION} — Hyprland ${HYPRLAND_VERSION}\""
echo "  5. git push && git push origin v${NEW_VERSION}"
