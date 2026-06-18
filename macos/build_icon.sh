#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Resolve directories
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

SVG_ICON="$PROJECT_ROOT/gitmaster.svg"
ICONSET_DIR="$SCRIPT_DIR/gitmaster.iconset"
ICNS_FILE="$SCRIPT_DIR/gitmaster.icns"

echo "Generating macOS application icon..."

if [ ! -f "$SVG_ICON" ]; then
    echo "Error: gitmaster.svg not found at $SVG_ICON"
    exit 1
fi

if ! command -v rsvg-convert &> /dev/null; then
    echo "Error: rsvg-convert is not installed. Please install librsvg."
    exit 1
fi

if ! command -v iconutil &> /dev/null; then
    echo "Error: iconutil is not available (macOS only tool)."
    exit 1
fi

# Clean up any previous temp dir
rm -rf "$ICONSET_DIR"
mkdir -p "$ICONSET_DIR"

# Generate the different required icon sizes for macOS
rsvg-convert -w 16 -h 16 "$SVG_ICON" -o "$ICONSET_DIR/icon_16x16.png"
rsvg-convert -w 32 -h 32 "$SVG_ICON" -o "$ICONSET_DIR/icon_16x16@2x.png"
rsvg-convert -w 32 -h 32 "$SVG_ICON" -o "$ICONSET_DIR/icon_32x32.png"
rsvg-convert -w 64 -h 64 "$SVG_ICON" -o "$ICONSET_DIR/icon_32x32@2x.png"
rsvg-convert -w 128 -h 128 "$SVG_ICON" -o "$ICONSET_DIR/icon_128x128.png"
rsvg-convert -w 256 -h 256 "$SVG_ICON" -o "$ICONSET_DIR/icon_128x128@2x.png"
rsvg-convert -w 256 -h 256 "$SVG_ICON" -o "$ICONSET_DIR/icon_256x256.png"
rsvg-convert -w 512 -h 512 "$SVG_ICON" -o "$ICONSET_DIR/icon_256x256@2x.png"
rsvg-convert -w 512 -h 512 "$SVG_ICON" -o "$ICONSET_DIR/icon_512x512.png"
rsvg-convert -w 1024 -h 1024 "$SVG_ICON" -o "$ICONSET_DIR/icon_512x512@2x.png"

# Package the iconset into an .icns file
iconutil -c icns "$ICONSET_DIR" -o "$ICNS_FILE"

# Clean up
rm -rf "$ICONSET_DIR"

echo "Successfully created icon: $ICNS_FILE"
