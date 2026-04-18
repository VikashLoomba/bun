#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PATCH_FILE="$ROOT/src/ios/patches/webkit-ios-device.patch"
WEBKIT_COMMIT="$(perl -ne 'print "$1\n" if /set\(WEBKIT_VERSION ([0-9a-f]{40})\)/' "$ROOT/cmake/tools/SetupWebKit.cmake")"

WEBKIT_SRC="${WEBKIT_SRC:-$ROOT/vendor/WebKit}"
WEBKIT_BUILD_DIR="${WEBKIT_BUILD_DIR:-$WEBKIT_SRC/build-ios-device}"
STAGED_WEBKIT_DIR="${STAGED_WEBKIT_DIR:-$ROOT/build/ios-webkit-device}"

if [ ! -d "$WEBKIT_SRC/.git" ]; then
  echo "WEBKIT_SRC must point to a full WebKit checkout. Got: $WEBKIT_SRC" >&2
  exit 1
fi

CURRENT_WEBKIT_COMMIT="$(git -C "$WEBKIT_SRC" rev-parse HEAD)"
if [ "$CURRENT_WEBKIT_COMMIT" != "$WEBKIT_COMMIT" ]; then
  echo "Expected WebKit commit $WEBKIT_COMMIT" >&2
  echo "Current WEBKIT_SRC commit is $CURRENT_WEBKIT_COMMIT" >&2
  echo "Run: git -C '$WEBKIT_SRC' checkout $WEBKIT_COMMIT" >&2
  exit 1
fi

if git -C "$WEBKIT_SRC" apply --reverse --check "$PATCH_FILE" >/dev/null 2>&1; then
  echo "WebKit device patch already applied"
else
  echo "Applying WebKit device patch"
  git -C "$WEBKIT_SRC" apply "$PATCH_FILE"
fi

cmake -S "$WEBKIT_SRC" -B "$WEBKIT_BUILD_DIR" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_SYSTEM_PROCESSOR=arm64 \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
  -DPORT=JSCOnly \
  -DENABLE_STATIC_JSC=ON \
  -DUSE_THIN_ARCHIVES=OFF \
  -DUSE_BUN_JSC_ADDITIONS=ON \
  -DUSE_BUN_EVENT_LOOP=ON \
  -DENABLE_BUN_SKIP_FAILING_ASSERTIONS=ON \
  -DALLOW_LINE_AND_COLUMN_NUMBER_IN_BUILTINS=ON \
  -DENABLE_JIT=OFF \
  -DENABLE_DFG_JIT=OFF \
  -DENABLE_FTL_JIT=OFF \
  -DENABLE_C_LOOP=ON \
  -DENABLE_SAMPLING_PROFILER=OFF \
  -DENABLE_WEBASSEMBLY=OFF \
  -DENABLE_REMOTE_INSPECTOR=ON

cmake --build "$WEBKIT_BUILD_DIR" --target JavaScriptCore

rm -rf "$STAGED_WEBKIT_DIR"
mkdir -p "$STAGED_WEBKIT_DIR/lib" "$STAGED_WEBKIT_DIR/include"

cp "$WEBKIT_BUILD_DIR/lib/"*.a "$STAGED_WEBKIT_DIR/lib/"
cp -R "$WEBKIT_BUILD_DIR/JavaScriptCore/Headers/"* "$STAGED_WEBKIT_DIR/include/"
cp "$WEBKIT_BUILD_DIR/cmakeconfig.h" "$STAGED_WEBKIT_DIR/include/"

mkdir -p \
  "$STAGED_WEBKIT_DIR/include/wtf" \
  "$STAGED_WEBKIT_DIR/include/bmalloc" \
  "$STAGED_WEBKIT_DIR/include/JavaScriptCore"

cp -R "$WEBKIT_BUILD_DIR/WTF/Headers/wtf/"* "$STAGED_WEBKIT_DIR/include/wtf/"
cp -R "$WEBKIT_BUILD_DIR/bmalloc/Headers/bmalloc/"* "$STAGED_WEBKIT_DIR/include/bmalloc/"
cp -R "$WEBKIT_BUILD_DIR/JavaScriptCore/PrivateHeaders/JavaScriptCore/"* "$STAGED_WEBKIT_DIR/include/JavaScriptCore/"

if [ -d "$WEBKIT_BUILD_DIR/JavaScriptCore/DerivedSources/inspector" ]; then
  cp -R "$WEBKIT_BUILD_DIR/JavaScriptCore/DerivedSources/inspector/"* "$STAGED_WEBKIT_DIR/include/JavaScriptCore/"
fi

cat > "$STAGED_WEBKIT_DIR/package.json" <<EOF
{
  "name": "bun-webkit-ios-device",
  "version": "$WEBKIT_COMMIT"
}
EOF

echo "Staged iOS device WebKit to: $STAGED_WEBKIT_DIR"
find "$STAGED_WEBKIT_DIR/lib" -maxdepth 1 -type f | sort
