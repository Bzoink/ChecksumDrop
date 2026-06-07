#!/usr/bin/env bash
# build-avalonia.sh — legacy .NET/Avalonia build (kept during the C migration)
#
# This is the original ChecksumDrop build script for the Avalonia app. It is
# retained while the C (raylib + raygui) port in ChecksumDropC/ is brought to
# parity. The top-level build.sh now builds the C port instead.

# Exit immediately if a command exits with a non-zero status
set -e

# Define directories
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_PATH="${SCRIPT_DIR}/ChecksumDrop.Avalonia/ChecksumDrop.Avalonia.csproj"
PUBLISH_DIR="${SCRIPT_DIR}/publish"

show_usage() {
    echo "Usage: $0 {linux|windows|clean}"
    echo "  linux   - Build and publish for Linux (linux-x64, framework-dependent)"
    echo "  windows - Cross-compile and publish for Windows (win-x64, framework-dependent)"
    echo "  clean   - Clean build and publish artifacts"
    exit 1
}

if [ $# -lt 1 ]; then
    show_usage
fi

case "$1" in
    linux)
        echo "Building ChecksumDrop for Linux (x64, framework-dependent)..."
        dotnet publish "${PROJECT_PATH}" \
            -c Release \
            -r linux-x64 \
            --self-contained false \
            -p:PublishSingleFile=true \
            -p:PublishReadyToRun=true \
            -o "${PUBLISH_DIR}/linux-x64"
        echo "Linux build completed successfully!"
        echo "Output directory: ${PUBLISH_DIR}/linux-x64"
        ;;
    windows)
        echo "Building ChecksumDrop for Windows (x64, cross-compilation, framework-dependent)..."
        dotnet publish "${PROJECT_PATH}" \
            -c Release \
            -r win-x64 \
            --self-contained false \
            -p:PublishSingleFile=true \
            -p:PublishReadyToRun=true \
            -o "${PUBLISH_DIR}/win-x64"
        echo "Windows build completed successfully!"
        echo "Output directory: ${PUBLISH_DIR}/win-x64"
        ;;
    clean)
        echo "Cleaning build and publish artifacts..."
        # Clean via dotnet
        dotnet clean "${PROJECT_PATH}" -c Release || true
        dotnet clean "${SCRIPT_DIR}/ChecksumDrop.Core/ChecksumDrop.Core.csproj" -c Release || true
        
        # Clean build output directories manually to ensure fresh builds
        echo "Removing bin, obj, and publish directories..."
        rm -rf "${PUBLISH_DIR}"
        rm -rf "${SCRIPT_DIR}/ChecksumDrop.Avalonia/bin" "${SCRIPT_DIR}/ChecksumDrop.Avalonia/obj"
        rm -rf "${SCRIPT_DIR}/ChecksumDrop.Core/bin" "${SCRIPT_DIR}/ChecksumDrop.Core/obj"
        
        echo "Clean completed successfully!"
        ;;
    *)
        show_usage
        ;;
esac
