# Building Bun for iOS Device

This document captures the reproducible iOS **device** build that was validated against the public `ios-port` branch and the public `pi-ios` history.

## Scope

This is the **device** path used for the `pi-ios` style build:

- target: `arm64-apple-ios`
- sysroot: `iphoneos`
- WebKit mode: `C_LOOP`
- JIT / DFG / FTL: disabled
- WebAssembly: disabled
- Sampling profiler: disabled
- remote inspector: enabled

## Exact WebKit input

Bun's `cmake/tools/SetupWebKit.cmake` currently pins WebKit to:

- `515344bc5d65aa2d4f9ff277b5fb944f0e051dcd`

The reproducible device build uses a **full** checkout of `oven-sh/WebKit` at that exact commit.

## Known incompatibilities

### 1. Public Bun-pinned WebKit checkout needs an iOS device patch

Without `src/ios/patches/webkit-ios-device.patch`, the device build fails in `Source/WTF/wtf/posix/OSAllocatorPOSIX.cpp` with errors like:

- `mach_vm.h unsupported`
- `use of undeclared identifier 'mach_vm_map'`

The patch in this repo is the same three-file fix recovered from the earlier `pi-ios` history.

### 2. Bun's pinned Zig bootstrap fails on Xcode 26.4 for the iOS device target

Bun currently pins Zig bootstrap commit:

- `c1423ff3fc7064635773a4a4616c5bf986eb00fe`

On Xcode 26.4, that bootstrap fails while building `bun-zig.o` for `aarch64-ios-none` with unresolved symbols such as:

- `__availability_version_check`
- `_abort`
- `_arc4random_buf`
- `_clock_gettime`
- `_dispatch_queue_create`
- `_fork`
- `_waitpid`

To make this reproducible, `cmake/tools/SetupZig.cmake` now supports:

- `-DZIG_PATH_OVERRIDE=/absolute/path/to/zig-bootstrap`

Use `scripts/install-zig-autobuild.sh` to install a verified working `oven-sh/zig` 0.15.2 autobuild and pass that path to the device build.

### 3. C_LOOP device builds disable WebAssembly and SamplingProfiler

The validated device WebKit configuration uses:

- `-DENABLE_C_LOOP=ON`
- `-DENABLE_WEBASSEMBLY=OFF`
- `-DENABLE_SAMPLING_PROFILER=OFF`

Bun now guards the affected codepaths so the device build fails closed instead of failing at compile or link time:

- CPU profiler APIs return empty/no-op results when `ENABLE(SAMPLING_PROFILER)` is off
- `compileStreaming()` / `instantiateStreaming()` reject with a runtime error when `ENABLE(WEBASSEMBLY)` is off
- `JSWasmStreamingCompiler.cpp` now compiles to an empty translation unit when WebAssembly is disabled

## Reproducible build

### 1. Prepare WebKit

```bash
git clone https://github.com/oven-sh/WebKit.git /absolute/path/to/WebKit
git -C /absolute/path/to/WebKit checkout 515344bc5d65aa2d4f9ff277b5fb944f0e051dcd
```

### 2. Install a working Zig bootstrap for Xcode 26.4

```bash
./scripts/install-zig-autobuild.sh
```

This prints an absolute path you can pass as `ZIG_PATH_OVERRIDE`.

### 3. Build Bun for iOS device

```bash
WEBKIT_SRC=/absolute/path/to/WebKit \
ZIG_PATH_OVERRIDE=/absolute/path/to/bootstrap-aarch64-macos-none \
./scripts/build-ios-device.sh
```

That script:

1. applies the iOS device patch to WebKit
2. configures and builds device JavaScriptCore
3. stages WebKit into `build/ios-webkit-device`
4. configures Bun with `cmake/toolchains/ios-device.cmake`
5. builds Bun into `build/ios-device`

## Packaging artifacts

To package a `pi-ios` style `libbun.a` bundle from the completed build:

```bash
./scripts/package-ios-device.sh
```

Default output:

- `build/ios-device-package/lib`
- `build/ios-device-package/include`

## Expected build artifacts

After a successful device build:

- `build/ios-webkit-device/lib/libJavaScriptCore.a`
- `build/ios-webkit-device/lib/libWTF.a`
- `build/ios-webkit-device/lib/libbmalloc.a`
- `build/ios-device/bun-zig.o`
- `build/ios-device/bun-profile.app/bun-profile`
