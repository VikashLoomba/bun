# Bun iOS Embedding API

This directory contains the iOS-specific code for embedding Bun in iOS applications.

## Overview

Bun can be embedded in iOS apps as a static library, providing a JavaScript runtime
with Node.js API compatibility. This is useful for apps that need to run JavaScript
code with file system access, network capabilities, and npm package support.

## Limitations

Due to iOS platform restrictions:

- **No JIT compilation**: JavaScriptCore runs in interpreter-only mode
- **No TinyCC/FFI**: Dynamic code generation is not available
- **Sandboxed filesystem**: Apps can only access their own container directories

## API

### Header: `bun_ios.h`

```c
#include "bun_ios.h"

// Callback type for when Bun exits
typedef void (*bun_exit_callback_t)(uint32_t exit_code);

// Start Bun with custom I/O file descriptors
// Returns 0 on success, -1 on error
int bun_start(
    int argc,
    const char** argv,
    int stdin_fd,      // File descriptor for stdin (-1 for /dev/null)
    int stdout_fd,     // File descriptor for stdout (-1 for /dev/null)
    int stderr_fd,     // File descriptor for stderr (-1 same as stdout, -2 keep default)
    bun_exit_callback_t on_exit
);

// Get Bun version string
const char* bun_version(void);
```

### Usage Example (Swift)

```swift
import Foundation

// Create pipes for I/O
let stdinPipe = Pipe()
let stdoutPipe = Pipe()

// Prepare arguments
let args = ["bun", "run", scriptPath]
var cArgs = args.map { strdup($0) }
cArgs.append(nil)

// Start Bun
let result = bun_start(
    Int32(args.count),
    &cArgs,
    stdinPipe.fileHandleForReading.fileDescriptor,
    stdoutPipe.fileHandleForWriting.fileDescriptor,
    -1,  // stderr same as stdout
    { exitCode in
        print("Bun exited with code: \(exitCode)")
    }
)

// Write to Bun's stdin
stdinPipe.fileHandleForWriting.write("input data".data(using: .utf8)!)

// Read from Bun's stdout
stdoutPipe.fileHandleForReading.readabilityHandler = { handle in
    let data = handle.availableData
    // Process output...
}
```

## Building

The iOS build requires:
- Xcode with iOS SDK
- CMake 3.20+
- Zig 0.15.2 bootstrap

### Simulator

```bash
cmake -B build-ios-sim \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/ios-simulator.cmake \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build-ios-sim
```

### Device (reproducible C_LOOP build)

```bash
git clone https://github.com/oven-sh/WebKit.git /absolute/path/to/WebKit
git -C /absolute/path/to/WebKit checkout 515344bc5d65aa2d4f9ff277b5fb944f0e051dcd

./scripts/install-zig-autobuild.sh

WEBKIT_SRC=/absolute/path/to/WebKit \
ZIG_PATH_OVERRIDE=/absolute/path/to/bootstrap-aarch64-macos-none \
./scripts/build-ios-device.sh
```

See [`BUILDING_DEVICE.md`](./BUILDING_DEVICE.md) for the exact WebKit patch, Zig bootstrap incompatibility, and staged header layout.

## Integration

1. Add `libbun.a` to your Xcode project
2. Add the header search path for `bun_ios.h`
3. Link required frameworks: `JavaScriptCore`, `Foundation`, `Security`
4. Add `-lc++` to Other Linker Flags

## Files

- `bun_ios.h` - Public API header
- `embedding.c` - Embedding API implementation (thread management, I/O redirection)
- `stubs.c` - Platform stubs for unavailable features (TCC, JIT)
- `webkit_stubs.cpp` - WebKit/JSC symbols missing from iOS builds
- `BUILDING_DEVICE.md` - Reproducible iOS device build path and incompatibilities
- `patches/webkit-ios-device.patch` - WebKit device patch recovered from the validated `pi-ios` path
