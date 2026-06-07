#!/usr/bin/env bash
# build.sh — ChecksumDrop top-level build helper
#
# ChecksumDrop is migrating from .NET/Avalonia to a C (raylib + raygui) port
# that lives in the ChecksumDropC/ subfolder. This script now forwards to that
# subfolder's build so the C app is the one built from the repo root.
#
# Usage: ./build.sh [clean|linux|windows|osx|all]
#
# When the migration is complete, the C port will move up to the repo root and
# replace the old Avalonia project entirely. The previous .NET build script is
# preserved at build-avalonia.sh for the duration of the transition.

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
C_BUILD="${SCRIPT_DIR}/ChecksumDropC/build.sh"

[[ -f "$C_BUILD" ]] || { echo "error: $C_BUILD not found" >&2; exit 1; }
[[ -x "$C_BUILD" ]] || chmod +x "$C_BUILD" 2>/dev/null || true

exec "$C_BUILD" "$@"
