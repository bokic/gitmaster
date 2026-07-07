#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Resolve directories
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Go to project root to ensure consistent build paths
cd "$PROJECT_ROOT"

# Ensure we are running on macOS
if [ "$(uname)" != "Darwin" ]; then
    echo "Error: This script must be run on macOS."
    exit 1
fi

echo "=== Starting GitMaster macOS Packaging Process ==="

# Check for Homebrew dependencies if brew is available
if command -v brew &> /dev/null; then
    echo "Checking required Homebrew packages..."
    if ! brew list libgit2 &>/dev/null; then
        echo "Warning: libgit2 not found via brew. Trying to continue, but build might fail."
        echo "Hint: Run 'brew install libgit2'"
    fi
    if ! brew list qt &>/dev/null; then
        echo "Warning: qt not found via brew. Trying to continue, but build might fail."
        echo "Hint: Run 'brew install qt'"
    fi
else
    echo "Warning: Homebrew not found. Please ensure dependencies are installed manually."
fi

# 1. Generate the macOS .icns icon if it doesn't exist
ICNS_FILE="$SCRIPT_DIR/gitmaster.icns"
if [ ! -f "$ICNS_FILE" ]; then
    echo "Application icon not found. Running build_icon.sh..."
    chmod +x "$SCRIPT_DIR/build_icon.sh"
    "$SCRIPT_DIR/build_icon.sh"
fi

# 2. Find macdeployqt
MACDEPLOYQT_PATH=$(which macdeployqt || true)
if [ -z "$MACDEPLOYQT_PATH" ]; then
    # Try Homebrew path
    if command -v brew &> /dev/null; then
        BREW_QT_PREFIX=$(brew --prefix qt 2>/dev/null || true)
        if [ -n "$BREW_QT_PREFIX" ] && [ -f "$BREW_QT_PREFIX/bin/macdeployqt" ]; then
            MACDEPLOYQT_PATH="$BREW_QT_PREFIX/bin/macdeployqt"
        fi
    fi
fi

if [ -z "$MACDEPLOYQT_PATH" ]; then
    # Standard fallback paths
    for path in "/opt/homebrew/bin/macdeployqt" "/usr/local/bin/macdeployqt"; do
        if [ -f "$path" ]; then
            MACDEPLOYQT_PATH="$path"
            break
        fi
    done
fi

if [ -z "$MACDEPLOYQT_PATH" ] || [ ! -f "$MACDEPLOYQT_PATH" ]; then
    echo "Error: macdeployqt could not be found. Please install Qt6 and add it to your PATH."
    exit 1
fi
echo "Using macdeployqt: $MACDEPLOYQT_PATH"

# 3. Compile the application in Release mode
echo "Configuring and building GitMaster (Release mode)..."
rm -rf build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Copy compile_commands.json to root (useful for IDEs)
cp build/compile_commands.json .

APP_BUNDLE="bin/gitmaster.app"
if [ ! -d "$APP_BUNDLE" ]; then
    echo "Error: App bundle not found at $APP_BUNDLE after build."
    exit 1
fi
echo "Successfully built: $APP_BUNDLE"

# 4. Deploy Qt and external library dependencies into the app bundle
echo "Running macdeployqt to bundle dependencies (Qt, libgit2, etc.)..."
"$MACDEPLOYQT_PATH" "$APP_BUNDLE" -verbose=1

# 4.5 Update version info in Info.plist to use the complete git describe version
GIT_DESCRIBE=$(git describe --tags --dirty 2>/dev/null || echo "latest")
echo "Updating Info.plist version strings to: $GIT_DESCRIBE"
chmod +w "$APP_BUNDLE/Contents/Info.plist"
plutil -replace CFBundleShortVersionString -string "$GIT_DESCRIBE" "$APP_BUNDLE/Contents/Info.plist"
plutil -replace CFBundleVersion -string "$GIT_DESCRIBE" "$APP_BUNDLE/Contents/Info.plist"
plutil -replace CFBundleDisplayName -string "Git Master" "$APP_BUNDLE/Contents/Info.plist"
plutil -replace CFBundleName -string "Git Master" "$APP_BUNDLE/Contents/Info.plist"

# 5. Resign the bundle
# macdeployqt copies dylibs from Homebrew which may have invalid/mismatched codesignatures.
# Deep codesigning using an ad-hoc signature (-) resolves this.
echo "Codesigning app bundle and its components..."
codesign --force --deep --sign - "$APP_BUNDLE"

# Verify signature
codesign --verify --verbose "$APP_BUNDLE"

# 6. Get version info for DMG filename
DMG_NAME="bin/GitMaster_${GIT_DESCRIBE}_macOS.dmg"

# 7. Package the app bundle into a DMG installer
echo "Creating DMG installer..."
DMG_DIR="build/dmg_contents"
rm -rf "$DMG_DIR"
mkdir -p "$DMG_DIR"

# Copy the app bundle to the DMG folder
cp -R "$APP_BUNDLE" "$DMG_DIR/"

# Create a symbolic link to Applications folder
ln -s /Applications "$DMG_DIR/Applications"

# Remove any existing DMG of the same name
rm -f "$DMG_NAME"

# Build the DMG
hdiutil create -volname "Git Master" -srcfolder "$DMG_DIR" -ov -format UDZO "$DMG_NAME"

# Clean up DMG staging directory
rm -rf "$DMG_DIR"

echo "=== Packaging Process Completed Successfully! ==="
echo "Artifact created: $DMG_NAME"
