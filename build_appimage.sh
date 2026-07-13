#!/usr/bin/env bash
set -euo pipefail

# Directories
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APPIMAGE_DIR="${PROJECT_DIR}/AppImage"
TOOLS_DIR="${APPIMAGE_DIR}/tools"
BUILD_DIR="${APPIMAGE_DIR}/build"
APPDIR="${APPIMAGE_DIR}/AppDir"

echo "=== GitMaster AppImage Packaging Pipeline ==="
echo "Project root: ${PROJECT_DIR}"
echo "AppImage workspace: ${APPIMAGE_DIR}"

# Create directories
mkdir -p "${APPIMAGE_DIR}"
mkdir -p "${TOOLS_DIR}"
mkdir -p "${BUILD_DIR}"

# 1. Download packaging tools
echo "=> Downloading packaging tools..."
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
QT_PLUGIN_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"

if [ ! -f "${TOOLS_DIR}/linuxdeploy-x86_64.AppImage" ]; then
    echo "Downloading linuxdeploy..."
    curl -L -o "${TOOLS_DIR}/linuxdeploy-x86_64.AppImage" "${LINUXDEPLOY_URL}"
    chmod +x "${TOOLS_DIR}/linuxdeploy-x86_64.AppImage"
else
    echo "linuxdeploy already exists."
fi

if [ ! -f "${TOOLS_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
    echo "Downloading linuxdeploy-plugin-qt..."
    curl -L -o "${TOOLS_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage" "${QT_PLUGIN_URL}"
    chmod +x "${TOOLS_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage"
else
    echo "linuxdeploy-plugin-qt already exists."
fi

# Ensure plugin symlink exists so linuxdeploy can find it via PATH
ln -sf linuxdeploy-plugin-qt-x86_64.AppImage "${TOOLS_DIR}/linuxdeploy-plugin-qt"

# 2. Build the project in Release mode
echo "=> Configuring and building gitmaster in Release mode..."
cmake -B "${BUILD_DIR}" -S "${PROJECT_DIR}" -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build "${BUILD_DIR}"

# 3. Install to temporary AppDir
echo "=> Recreating AppDir staging directory..."
rm -rf "${APPDIR}"
mkdir -p "${APPDIR}"
cmake --install "${BUILD_DIR}" --prefix "${APPDIR}/usr"

# 4. Packaging the AppImage
echo "=> Executing linuxdeploy..."

# Set version
if [ -z "${VERSION:-}" ]; then
    # Get version from git tags
    VERSION=$(git describe --tags --always || echo "1.0.0")
    # Clean version string (remove 'v' prefix if present)
    VERSION="${VERSION#v}"
    export VERSION
fi
echo "AppImage Version: ${VERSION}"

# Environment configuration for Qt6 plugin detection
export QMAKE="/usr/lib/qt6/bin/qmake"
export PATH="${TOOLS_DIR}:${PATH}"
export NO_STRIP=true

# We run linuxdeploy from inside the AppImage directory so the output AppImage lands there
cd "${APPIMAGE_DIR}"

"${TOOLS_DIR}/linuxdeploy-x86_64.AppImage" \
    --appdir "${APPDIR}" \
    --executable "${APPDIR}/usr/bin/gitmaster" \
    --desktop-file "${PROJECT_DIR}/gitmaster.desktop" \
    --icon-file "${PROJECT_DIR}/gitmaster.svg" \
    --plugin qt \
    --output appimage

echo "=> AppImage build complete!"
ls -lh "${APPIMAGE_DIR}"/*.AppImage
