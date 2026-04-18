#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUN_BUILD_DIR="${BUN_BUILD_DIR:-$ROOT/build/ios-device}"
STAGED_WEBKIT_DIR="${STAGED_WEBKIT_DIR:-$ROOT/build/ios-webkit-device}"
OUT_DIR="${OUT_DIR:-$ROOT/build/ios-device-package}"

if [ ! -d "$BUN_BUILD_DIR" ]; then
  echo "BUN_BUILD_DIR does not exist: $BUN_BUILD_DIR" >&2
  exit 1
fi

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR/lib" "$OUT_DIR/include"

BUN_OBJECTS=$(find "$BUN_BUILD_DIR/CMakeFiles/bun-profile.dir" -name '*.o' -type f | sort)
ar rcs "$OUT_DIR/lib/libbun.a" $BUN_OBJECTS "$BUN_BUILD_DIR/bun-zig.o"

cp "$BUN_BUILD_DIR/mimalloc/CMakeFiles/mimalloc-obj.dir/src/static.c.o" "$OUT_DIR/lib/mimalloc.o"
cp "$STAGED_WEBKIT_DIR/lib/"*.a "$OUT_DIR/lib/"
cp -R "$STAGED_WEBKIT_DIR/include/"* "$OUT_DIR/include/"
cp "$ROOT/src/ios/bun_ios.h" "$OUT_DIR/include/"

for lib in \
  boringssl/libcrypto.a \
  boringssl/libssl.a \
  boringssl/libdecrepit.a \
  brotli/libbrotlicommon.a \
  brotli/libbrotlidec.a \
  brotli/libbrotlienc.a \
  cares/lib/libcares.a \
  highway/libhwy.a \
  libdeflate/libdeflate.a \
  lolhtml/aarch64-apple-ios/release/liblolhtml.a \
  lshpack/libls-hpack.a \
  zlib/libz.a \
  libarchive/libarchive/libarchive.a \
  hdrhistogram/src/libhdr_histogram_static.a \
  zstd/lib/libzstd.a

do
  cp "$BUN_BUILD_DIR/$lib" "$OUT_DIR/lib/"
done

echo "Packaged Bun iOS device libraries to: $OUT_DIR"
ls -lh "$OUT_DIR/lib"
