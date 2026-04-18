# Bun iOS Port - Changes from upstream

This document tracks all changes made to support iOS embedding.

## New Files (src/ios/)

| File | Purpose |
|------|---------|
| `bun_ios.h` | Public C API for embedding Bun in iOS apps |
| `embedding.c` | Thread management, I/O redirection implementation |
| `stubs.c` | TCC stubs (JIT unavailable on iOS) |
| `webkit_stubs.cpp` | JSC debug symbols (missing from iOS build) |
| `README.md` | Documentation for iOS embedders |
| `BUILDING_DEVICE.md` | Reproducible iOS device build path and incompatibilities |
| `patches/webkit-ios-device.patch` | WebKit patch required for the public device build |

## Build System Changes

| File | Change |
|------|--------|
| `cmake/toolchains/ios-simulator.cmake` | New toolchain for iOS Simulator builds |
| `cmake/toolchains/ios-device.cmake` | Toolchain for reproducible iOS device builds |
| `cmake/tools/SetupZig.cmake` | Supports `ZIG_PATH_OVERRIDE` for Xcode 26.4-compatible Zig bootstraps |
| `cmake/targets/BuildBun.cmake` | Added iOS source files and iOS device Zig sysroot selection |
| `cmake/targets/BuildLolHtml.cmake` | Uses `aarch64-apple-ios` for device builds |
| `cmake/Options.cmake` | iOS detection and options |
| `cmake/Globals.cmake` | iOS target configuration |
| `cmake/CompilerFlags.cmake` | iOS compiler flags |
| `build.zig` | iOS target support |
| `scripts/install-zig-autobuild.sh` | Installs a verified working Zig 0.15.2 autobuild |
| `scripts/build-ios-device-webkit.sh` | Applies the WebKit device patch, builds JSC, and stages headers/libs |
| `scripts/build-ios-device.sh` | Reproducible Bun iOS device build helper |
| `scripts/package-ios-device.sh` | Packages `libbun.a` and its dependent static libraries |

## Runtime Changes

### Platform Detection (`src/env.zig`)
- Added `isIOS` constant
- Changed `isDarwin` to include iOS (`isMac or isIOS`)
- Added iOS to `OperatingSystem` enum

### Entry Point (`src/main.zig`, `src/bun.zig`)
- Refactored `main()` to call `bunMain()` 
- Added `export fn bun_main()` for iOS embedding
- Added `bun_ios_argc`/`bun_ios_argv` for argument passing

### Exit Handling (`src/Global.zig`)
- Added `bun_ios_exit_callback` for exit notifications
- iOS calls callback instead of `exit()` to keep host app alive

### DNS Resolution (`src/bun.js/api/bun/dns.zig`)
- iOS skips libinfo DNS, uses work pool instead
- Reason: uSockets loop issue with libinfo on HTTP thread

### Disabled Features
- TCC/FFI JIT compilation (no JIT on iOS)
- File system watching (limited on iOS)
- Process spawning (sandboxed on iOS)
- Sampling profiler in the validated device C_LOOP build (`ENABLE_SAMPLING_PROFILER=OFF`)
- WebAssembly streaming in the validated device C_LOOP build (`ENABLE_WEBASSEMBLY=OFF`)

## How to Upstream

When submitting PR to oven-sh/bun:

1. **First PR**: Just `src/ios/` directory + build system changes
2. **Second PR**: Runtime changes with detailed explanations
3. **Document**: All iOS limitations clearly

## Testing

### Unit tests (run on any platform)
```bash
bun test test/ios/
```

### Build for iOS Simulator
```bash
cmake -B build-ios-sim \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/ios-simulator.cmake \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build-ios-sim
```

### Build for iOS Device
```bash
./scripts/install-zig-autobuild.sh
WEBKIT_SRC=/absolute/path/to/WebKit \
ZIG_PATH_OVERRIDE=/absolute/path/to/bootstrap-aarch64-macos-none \
./scripts/build-ios-device.sh
```

### Integration testing
Create a minimal iOS app that embeds `libbun.a` and calls the `bun_start()` API.
See `test/ios/` for example usage patterns.
