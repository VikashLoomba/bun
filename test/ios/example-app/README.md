# Bun iOS Example App

This is a minimal example showing how to embed Bun in an iOS application.

## Structure

```
example-app/
├── BunExample/
│   ├── BunExample.xcodeproj/
│   ├── BunBridge.h          # C bridging header
│   ├── BunRuntime.swift     # Swift wrapper for Bun API
│   ├── ContentView.swift    # SwiftUI interface
│   └── scripts/
│       └── hello.js         # Example JavaScript to run
├── build.sh                 # Build script
└── README.md
```

## Prerequisites

1. Build Bun for iOS Simulator:
   ```bash
   cd /path/to/bun
   cmake -B build-ios-sim \
       -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/ios-simulator.cmake \
       -DCMAKE_BUILD_TYPE=Release
   cmake --build build-ios-sim
   ```

2. Xcode 15+ with iOS 17 SDK

## Building

```bash
./build.sh
```

This will:
1. Copy `libbun.a` and headers from the Bun build
2. Build the Xcode project
3. Install to the iOS Simulator

## Running

```bash
# Boot simulator if needed
xcrun simctl boot "iPhone 15 Pro"

# Launch the app
xcrun simctl launch --console booted com.example.BunExample
```

## How It Works

### BunBridge.h
```c
#include "bun_ios.h"
```

### BunRuntime.swift
```swift
class BunRuntime {
    func runScript(_ path: String, completion: @escaping (Int32) -> Void) {
        // Set up pipes for I/O
        let stdin = Pipe()
        let stdout = Pipe()
        
        // Call bun_start() with redirected I/O
        bun_start(argc, argv, stdin.fd, stdout.fd, -1, callback)
    }
}
```

### ContentView.swift
```swift
struct ContentView: View {
    @State private var output = ""
    let runtime = BunRuntime()
    
    var body: some View {
        VStack {
            Text(output)
            Button("Run Script") {
                runtime.runScript("hello.js") { code in
                    output = "Exited with code: \(code)"
                }
            }
        }
    }
}
```

## Troubleshooting

### "Library not found"
Ensure `libbun.a` is in the project's library search paths.

### "Symbol not found"
Add `-lc++` to Other Linker Flags in Xcode.

### Sandbox errors
Use the app's Documents directory for any file operations:
```swift
let docs = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
```
