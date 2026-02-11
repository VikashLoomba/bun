#!/bin/bash
#
# Run Bun iOS tests in the iOS Simulator
#
# Usage:
#   ./scripts/ios-test.sh              # Run all iOS tests
#   ./scripts/ios-test.sh runtime      # Run only runtime tests
#   ./scripts/ios-test.sh embedding    # Run only embedding API tests
#
# Prerequisites:
#   - Xcode with iOS Simulator
#   - iOS build of Bun (run cmake build first)
#   - Booted iOS Simulator (or script will boot one)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUN_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUN_ROOT}/build/ios-release"
TEST_DIR="${BUN_ROOT}/test/ios"
BUNDLE_ID="sh.oven.bun.ios-test"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log() { echo -e "${GREEN}[ios-test]${NC} $1"; }
warn() { echo -e "${YELLOW}[ios-test]${NC} $1"; }
error() { echo -e "${RED}[ios-test]${NC} $1" >&2; }

# Check prerequisites
check_prerequisites() {
    if ! command -v xcrun &> /dev/null; then
        error "Xcode command line tools not found. Install with: xcode-select --install"
        exit 1
    fi
    
    if [ ! -f "${BUILD_DIR}/bun-profile.app/bun-profile" ]; then
        error "iOS build not found at ${BUILD_DIR}"
        error "Build with: cmake -B build-ios-release -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/ios-simulator.cmake && cmake --build build-ios-release"
        exit 1
    fi
    
    log "Prerequisites OK"
}

# Get or boot a simulator
get_simulator() {
    # Check for already booted simulator
    BOOTED=$(xcrun simctl list devices booted -j | grep -o '"udid" : "[^"]*"' | head -1 | cut -d'"' -f4)
    
    if [ -n "$BOOTED" ]; then
        log "Using booted simulator: $BOOTED"
        echo "$BOOTED"
        return
    fi
    
    # Find an available iPhone simulator
    DEVICE=$(xcrun simctl list devices available -j | grep -E '"iPhone [0-9]+ Pro"' -A1 | grep udid | head -1 | cut -d'"' -f4)
    
    if [ -z "$DEVICE" ]; then
        # Fallback to any iPhone
        DEVICE=$(xcrun simctl list devices available -j | grep -E '"iPhone' -A1 | grep udid | head -1 | cut -d'"' -f4)
    fi
    
    if [ -z "$DEVICE" ]; then
        error "No iPhone simulator found. Create one in Xcode."
        exit 1
    fi
    
    log "Booting simulator: $DEVICE"
    xcrun simctl boot "$DEVICE" 2>/dev/null || true
    
    # Wait for boot
    sleep 3
    
    echo "$DEVICE"
}

# Build the test runner app
build_test_app() {
    log "Building iOS test runner app..."
    
    local APP_DIR="${BUILD_DIR}/ios-test.app"
    mkdir -p "$APP_DIR"
    
    # Copy Bun binary
    cp "${BUILD_DIR}/bun-profile.app/bun-profile" "$APP_DIR/bun"
    
    # Copy test files
    cp -r "$TEST_DIR"/*.ts "$APP_DIR/" 2>/dev/null || true
    cp -r "$TEST_DIR"/*.js "$APP_DIR/" 2>/dev/null || true
    
    # Create minimal Info.plist
    cat > "$APP_DIR/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleIdentifier</key>
    <string>${BUNDLE_ID}</string>
    <key>CFBundleName</key>
    <string>BunIOSTest</string>
    <key>CFBundleExecutable</key>
    <string>bun</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>LSRequiresIPhoneOS</key>
    <true/>
    <key>UIRequiredDeviceCapabilities</key>
    <array>
        <string>arm64</string>
    </array>
    <key>MinimumOSVersion</key>
    <string>17.0</string>
</dict>
</plist>
EOF
    
    # Sign for simulator (ad-hoc)
    codesign --force --sign - "$APP_DIR"
    
    log "Test app built: $APP_DIR"
    echo "$APP_DIR"
}

# Run tests in simulator
run_tests() {
    local SIMULATOR=$1
    local TEST_FILTER=$2
    local APP_DIR=$3
    
    log "Installing test app..."
    xcrun simctl install "$SIMULATOR" "$APP_DIR"
    
    log "Running tests..."
    
    # Prepare test command
    local TEST_CMD="test"
    if [ -n "$TEST_FILTER" ]; then
        TEST_CMD="test test/ios/${TEST_FILTER}.test.ts"
    else
        TEST_CMD="test test/ios/"
    fi
    
    # Launch and capture output
    # Note: simctl launch --console captures stdout/stderr
    xcrun simctl launch --console "$SIMULATOR" "$BUNDLE_ID" "$TEST_CMD" 2>&1 | tee /tmp/bun-ios-test.log
    
    # Check for failures in output
    if grep -q "fail\|FAIL\|Error:" /tmp/bun-ios-test.log; then
        error "Some tests failed!"
        return 1
    fi
    
    log "All tests passed!"
    return 0
}

# Cleanup
cleanup() {
    local SIMULATOR=$1
    log "Cleaning up..."
    xcrun simctl uninstall "$SIMULATOR" "$BUNDLE_ID" 2>/dev/null || true
}

# Main
main() {
    local TEST_FILTER="${1:-}"
    
    log "Starting iOS tests..."
    
    check_prerequisites
    
    SIMULATOR=$(get_simulator)
    APP_DIR=$(build_test_app)
    
    trap "cleanup $SIMULATOR" EXIT
    
    if run_tests "$SIMULATOR" "$TEST_FILTER" "$APP_DIR"; then
        log "✅ iOS tests completed successfully"
        exit 0
    else
        error "❌ iOS tests failed"
        exit 1
    fi
}

main "$@"
