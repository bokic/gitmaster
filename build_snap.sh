#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== GitMaster Snap Packaging Pipeline ==="
echo "Project root: ${PROJECT_DIR}"

# Check that snapcraft is installed
if ! command -v snapcraft &>/dev/null; then
    echo "Error: snapcraft is not installed."
    echo "Install it with: sudo snap install snapcraft --classic"
    exit 1
fi

cd "${PROJECT_DIR}"

echo "=> Building .snap package..."
snapcraft

echo ""
echo "=> Snap build complete!"
ls -lh "${PROJECT_DIR}"/*.snap

echo ""
echo "To install locally, run:"
echo "  sudo snap install --dangerous $(ls "${PROJECT_DIR}"/*.snap | tail -1)"
