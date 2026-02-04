set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_SYSTEM_PROCESSOR arm64)
set(CMAKE_OSX_SYSROOT iphonesimulator)
set(CMAKE_OSX_ARCHITECTURES arm64)
set(CMAKE_OSX_DEPLOYMENT_TARGET 16.0)

set(CMAKE_C_COMPILER_TARGET arm64-apple-ios16.0-simulator)
set(CMAKE_CXX_COMPILER_TARGET arm64-apple-ios16.0-simulator)

execute_process(
    COMMAND xcrun --sdk iphonesimulator --show-sdk-path
    OUTPUT_VARIABLE IOS_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(ENV{IOS_SYSROOT} "${IOS_SDK_PATH}")
