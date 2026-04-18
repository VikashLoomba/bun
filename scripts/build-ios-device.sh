#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

unset SDKROOT
unset MACOSX_DEPLOYMENT_TARGET
unset CMAKE_OSX_SYSROOT
unset CMAKE_OSX_DEPLOYMENT_TARGET
unset CC
unset CXX

WEBKIT_SRC="${WEBKIT_SRC:-$ROOT/vendor/WebKit}"
STAGED_WEBKIT_DIR="${STAGED_WEBKIT_DIR:-$ROOT/build/ios-webkit-device}"
BUN_BUILD_DIR="${BUN_BUILD_DIR:-$ROOT/build/ios-device}"
CMAKE_GENERATOR="${CMAKE_GENERATOR:-Ninja}"
ZIG_PATH_OVERRIDE="${ZIG_PATH_OVERRIDE:-}"

if [ -z "$WEBKIT_SRC" ] || [ ! -d "$WEBKIT_SRC" ]; then
  echo "Set WEBKIT_SRC to a full WebKit checkout" >&2
  exit 1
fi

xcode-select -p >/dev/null
xcrun --sdk iphoneos --show-sdk-path >/dev/null
rustup target add aarch64-apple-ios >/dev/null

WEBKIT_SRC="$WEBKIT_SRC" STAGED_WEBKIT_DIR="$STAGED_WEBKIT_DIR" "$SCRIPT_DIR/build-ios-device-webkit.sh"

cmake_args=(
  -S "$ROOT"
  -B "$BUN_BUILD_DIR"
  -G "$CMAKE_GENERATOR"
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchains/ios-device.cmake"
  -DWEBKIT_PATH="$STAGED_WEBKIT_DIR"
)

if [ -n "$ZIG_PATH_OVERRIDE" ]; then
  cmake_args+=("-DZIG_PATH_OVERRIDE=$ZIG_PATH_OVERRIDE")
fi

cmake "${cmake_args[@]}"
cmake --build "$BUN_BUILD_DIR" --target clone-zig
cmake --build "$BUN_BUILD_DIR"

echo "Built Bun iOS device artifacts:"
echo "  $BUN_BUILD_DIR/bun-zig.o"
echo "  $BUN_BUILD_DIR/bun-profile.app/bun-profile"
