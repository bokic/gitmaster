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

# Set the output version. Keep VERSION as a supported input for callers, but
# pass it to linuxdeploy using its current variable name.
if [ -z "${LINUXDEPLOY_OUTPUT_VERSION:-}" ]; then
    if [ -n "${VERSION:-}" ]; then
        LINUXDEPLOY_OUTPUT_VERSION="${VERSION}"
    else
        LINUXDEPLOY_OUTPUT_VERSION=$(git describe --tags --always || echo "1.0.0")
    fi
    LINUXDEPLOY_OUTPUT_VERSION="${LINUXDEPLOY_OUTPUT_VERSION#v}"
    export LINUXDEPLOY_OUTPUT_VERSION
fi
echo "AppImage Version: ${LINUXDEPLOY_OUTPUT_VERSION}"

# Environment configuration for Qt plugin detection
if [ -z "${QMAKE:-}" ]; then
    if [ -x "/usr/lib/qt6/bin/qmake" ]; then
        export QMAKE="/usr/lib/qt6/bin/qmake"
    elif command -v qmake6 >/dev/null 2>&1; then
        export QMAKE="$(command -v qmake6)"
    elif command -v qmake >/dev/null 2>&1; then
        export QMAKE="$(command -v qmake)"
    fi
fi
export PATH="${TOOLS_DIR}:${PATH}"
export NO_STRIP=true

# Detect available Wayland platform plugins so modern Linux distros (Fedora, Arch, etc.)
# can run on Wayland natively with graceful fallback to XCB (X11)
QT_PLUGINS_DIR=""
if [ -n "${QMAKE:-}" ]; then
    QT_PLUGINS_DIR="$("${QMAKE}" -query QT_INSTALL_PLUGINS 2>/dev/null || echo "")"
fi
EXTRA_PLUGINS=()
for plugin in libqwayland.so libqwayland-egl.so libqwayland-generic.so; do
    if [ -n "${QT_PLUGINS_DIR}" ] && [ -f "${QT_PLUGINS_DIR}/platforms/${plugin}" ]; then
        EXTRA_PLUGINS+=("${plugin}")
    fi
done
if [ ${#EXTRA_PLUGINS[@]} -gt 0 ]; then
    export EXTRA_PLATFORM_PLUGINS="$(IFS=';'; echo "${EXTRA_PLUGINS[*]}")"
    echo "Configured EXTRA_PLATFORM_PLUGINS: ${EXTRA_PLATFORM_PLUGINS}"
fi
DEPLOY_DEPS_ARGS=()
if [ -n "${QT_PLUGINS_DIR}" ] && [ -d "${QT_PLUGINS_DIR}/wayland-graphics-integration-client" ]; then
    mkdir -p "${APPDIR}/usr/plugins/wayland-graphics-integration-client"
    cp -a "${QT_PLUGINS_DIR}/wayland-graphics-integration-client/"*.so "${APPDIR}/usr/plugins/wayland-graphics-integration-client/"
    DEPLOY_DEPS_ARGS+=(--deploy-deps-only "${APPDIR}/usr/plugins/wayland-graphics-integration-client")
fi

# We run linuxdeploy from inside the AppImage directory so the output AppImage lands there
cd "${APPIMAGE_DIR}"

# 4a. Run linuxdeploy with Qt plugin to bundle dependencies and plugins
echo "=> Deploying dependencies and Qt plugins into AppDir..."
"${TOOLS_DIR}/linuxdeploy-x86_64.AppImage" \
    --appdir "${APPDIR}" \
    --executable "${APPDIR}/usr/bin/gitmaster" \
    --desktop-file "${PROJECT_DIR}/io.github.bokic.gitmaster.desktop" \
    --icon-file "${PROJECT_DIR}/io.github.bokic.gitmaster.svg" \
    "${DEPLOY_DEPS_ARGS[@]}" \
    --plugin qt

# 4b. Ensure AppRun script and qt.conf files are properly configured
# linuxdeploy creates a direct symlink AppRun -> usr/bin/gitmaster and skips AppRun hook
# on Qt 6. In addition, Qt resolves qt.conf relative to argv[0] (which is AppDir root when
# launched by AppImage runtime). Without an AppRun wrapper and root qt.conf, Qt fails to
# discover platform plugins (such as wayland or xcb).
echo "=> Configuring AppRun launcher and qt.conf..."
rm -f "${APPDIR}/AppRun"
cat << 'EOF' > "${APPDIR}/AppRun"
#!/usr/bin/env bash
HERE="$(dirname "$(readlink -f "${0}")")"
export PATH="${HERE}/usr/bin${PATH:+:${PATH}}"
export LD_LIBRARY_PATH="${HERE}/usr/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export QT_PLUGIN_PATH="${HERE}/usr/plugins${QT_PLUGIN_PATH:+:${QT_PLUGIN_PATH}}"
export QML2_IMPORT_PATH="${HERE}/usr/qml${QML2_IMPORT_PATH:+:${QML2_IMPORT_PATH}}"
export XDG_DATA_DIRS="${HERE}/usr/share${XDG_DATA_DIRS:+:${XDG_DATA_DIRS}}"
exec "${HERE}/usr/bin/gitmaster" "$@"
EOF
chmod +x "${APPDIR}/AppRun"

cat << 'EOF' > "${APPDIR}/qt.conf"
[Paths]
Prefix = usr
Plugins = plugins
Imports = qml
Qml2Imports = qml
EOF

if [ ! -f "${APPDIR}/usr/bin/qt.conf" ]; then
    cat << 'EOF' > "${APPDIR}/usr/bin/qt.conf"
[Paths]
Prefix = ../
Plugins = plugins
Imports = qml
Qml2Imports = qml
EOF
fi

# 4c. Generate the final AppImage package
echo "=> Generating final AppImage package..."
"${TOOLS_DIR}/linuxdeploy-x86_64.AppImage" \
    --appdir "${APPDIR}" \
    --output appimage

echo "=> AppImage build complete!"
ls -lh "${APPIMAGE_DIR}"/*.AppImage
