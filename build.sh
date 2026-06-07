#!/usr/bin/env bash
# build.sh — ChecksumDrop (C / raylib + raygui) build helper
# Usage: ./build.sh [clean|linux|windows|osx|all]
#
# linux   — native Debug build for the current Linux machine
# windows — cross-compile Release for Windows x64 via mingw-w64
#           requires: mingw-w64, NSIS (makensis) for the installer
# osx     — cross-compile Release for macOS via osxcross toolchain
#           requires: osxcross installed and OSXCROSS_ROOT set
# all     — linux + windows + osx
# clean   — remove all build directories

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR"
BUILD_ROOT="$SCRIPT_DIR/build"
NPROC=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)

# ── helpers ──────────────────────────────────────────────────────────────────

die() { echo "error: $*" >&2; exit 1; }

require_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "'$1' not found — install it first"
}

build_native_linux() {
    echo "==> Building Linux (native, Debug)"
    local build_dir="$BUILD_ROOT/linux"
    cmake -S "$SRC_DIR" -B "$build_dir" \
        -DCMAKE_BUILD_TYPE=Debug
    cmake --build "$build_dir" -j"$NPROC"
    echo "==> Output: $build_dir/ChecksumDrop"
}

build_windows() {
    echo "==> Building Windows x64 (cross, Release + NSIS installer)"
    require_cmd x86_64-w64-mingw32-gcc
    require_cmd cpack
    local build_dir="$BUILD_ROOT/windows"
    local toolchain="$SCRIPT_DIR/cmake/toolchain-mingw64.cmake"
    [[ -f "$toolchain" ]] || die "Toolchain file not found: $toolchain"
    cmake -S "$SRC_DIR" -B "$build_dir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$toolchain"
    cmake --build "$build_dir" -j"$NPROC"

    # Generate Windows installer (.exe) with NSIS.
    cpack -G NSIS --config "$build_dir/CPackConfig.cmake" -B "$build_dir"

    local installer
    installer=$(ls -1 "$build_dir"/*.exe 2>/dev/null | grep -E 'ChecksumDrop-.*-win64\.exe$' | head -n1 || true)

    echo "==> Output (app): $build_dir/ChecksumDrop.exe"
    if [[ -n "$installer" ]]; then
        echo "==> Output (installer): $installer"
    else
        echo "==> Output (installer): check $build_dir for ChecksumDrop-*-win64.exe"
    fi
}

build_osx() {
    echo "==> Building macOS arm64 (cross, Release)"
    : "${OSXCROSS_ROOT:?Set OSXCROSS_ROOT to your osxcross installation directory}"
    local build_dir="$BUILD_ROOT/osx"
    local toolchain="$SCRIPT_DIR/cmake/toolchain-osxcross.cmake"
    [[ -f "$toolchain" ]] || die "Toolchain file not found: $toolchain"
    cmake -S "$SRC_DIR" -B "$build_dir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
        -DOSXCROSS_ROOT="$OSXCROSS_ROOT"
    cmake --build "$build_dir" -j"$NPROC"
    echo "==> Output: $build_dir/ChecksumDrop"
}

do_clean() {
    echo "==> Cleaning build directories"
    rm -rf "$BUILD_ROOT"
    echo "==> Done"
}

# ── main ─────────────────────────────────────────────────────────────────────

TARGET="${1:-linux}"

case "$TARGET" in
    clean)   do_clean ;;
    linux)   build_native_linux ;;
    windows) build_windows ;;
    osx)     build_osx ;;
    all)
        build_native_linux
        build_windows
        build_osx
        ;;
    *)
        echo "Usage: $0 [clean|linux|windows|osx|all]"
        echo
        echo "  clean    Remove all build directories"
        echo "  linux    Native Debug build (default)"
        echo "  windows  Cross-compile Release for Windows x64 (requires mingw-w64)"
        echo "  osx      Cross-compile Release for macOS (requires osxcross + OSXCROSS_ROOT)"
        echo "  all      linux + windows + osx"
        exit 1
        ;;
esac
