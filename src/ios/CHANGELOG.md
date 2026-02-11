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

## Build System Changes

| File | Change |
|------|--------|
| `cmake/toolchains/ios-simulator.cmake` | New toolchain for iOS Simulator builds |
| `cmake/targets/BuildBun.cmake` | Added iOS source files |
| `cmake/Options.cmake` | iOS detection and options |
| `cmake/Globals.cmake` | iOS target configuration |
| `cmake/CompilerFlags.cmake` | iOS compiler flags |
| `build.zig` | iOS target support |

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

## How to Upstream

When submitting PR to oven-sh/bun:

1. **First PR**: Just `src/ios/` directory + build system changes
2. **Second PR**: Runtime changes with detailed explanations
3. **Document**: All iOS limitations clearly

## Testing

Build for iOS Simulator:
```bash
cmake -B build-ios-sim \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/ios-simulator.cmake \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build-ios-sim
```

Run tests in iOS Simulator app (see pi-terminal repo).
