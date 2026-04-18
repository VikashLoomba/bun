#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

ZIG_AUTOBUILD_TAG="${ZIG_AUTOBUILD_TAG:-autobuild-f6231adad307ef52dc390eb3aa6db844d1b261a3}"
ZIG_INSTALL_ROOT="${ZIG_INSTALL_ROOT:-$ROOT/build/zig-autobuild/$ZIG_AUTOBUILD_TAG}"

uname_s="$(uname -s)"
uname_m="$(uname -m)"

case "$uname_s-$uname_m" in
  Darwin-arm64)
    ZIG_ARCHIVE_NAME="bootstrap-aarch64-macos-none.zip"
    ZIG_EXTRACTED_DIR="bootstrap-aarch64-macos-none"
    ;;
  Darwin-x86_64)
    ZIG_ARCHIVE_NAME="bootstrap-x86_64-macos-none.zip"
    ZIG_EXTRACTED_DIR="bootstrap-x86_64-macos-none"
    ;;
  Linux-aarch64|Linux-arm64)
    ZIG_ARCHIVE_NAME="bootstrap-aarch64-linux-musl.zip"
    ZIG_EXTRACTED_DIR="bootstrap-aarch64-linux-musl"
    ;;
  Linux-x86_64)
    ZIG_ARCHIVE_NAME="bootstrap-x86_64-linux-musl.zip"
    ZIG_EXTRACTED_DIR="bootstrap-x86_64-linux-musl"
    ;;
  *)
    echo "Unsupported host for this helper: $uname_s-$uname_m" >&2
    exit 1
    ;;
esac

DOWNLOAD_URL="https://github.com/oven-sh/zig/releases/download/${ZIG_AUTOBUILD_TAG}/${ZIG_ARCHIVE_NAME}"
ARCHIVE_PATH="$ZIG_INSTALL_ROOT/$ZIG_ARCHIVE_NAME"
EXTRACT_PATH="$ZIG_INSTALL_ROOT/$ZIG_EXTRACTED_DIR"

mkdir -p "$ZIG_INSTALL_ROOT"

if [ ! -x "$EXTRACT_PATH/zig" ]; then
  echo "Downloading $DOWNLOAD_URL"
  curl -fL "$DOWNLOAD_URL" -o "$ARCHIVE_PATH"
  rm -rf "$EXTRACT_PATH"
  unzip -q "$ARCHIVE_PATH" -d "$ZIG_INSTALL_ROOT"
fi

echo "Installed Zig bootstrap to: $EXTRACT_PATH"
echo "zig version: $($EXTRACT_PATH/zig version)"
echo "Use it with: -DZIG_PATH_OVERRIDE=$EXTRACT_PATH"
