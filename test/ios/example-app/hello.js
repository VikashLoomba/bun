// Example JavaScript for Bun iOS
// This script demonstrates basic Bun functionality on iOS

console.log("Hello from Bun on iOS!");
console.log("Bun version:", Bun.version);
console.log("Platform:", process.platform);
console.log("Architecture:", process.arch);

// Test basic operations
const encoder = new TextEncoder();
const decoder = new TextDecoder();
const encoded = encoder.encode("Hello, 世界!");
const decoded = decoder.decode(encoded);
console.log("TextEncoder/Decoder:", decoded);

// Test crypto
const uuid = crypto.randomUUID();
console.log("Random UUID:", uuid);

// Test fetch (if network is available)
async function testFetch() {
  try {
    const response = await fetch("https://example.com");
    console.log("Fetch status:", response.status);
  } catch (error) {
    console.log("Fetch error (expected if no network):", error.message);
  }
}

// Test file operations (in sandbox)
async function testFiles() {
  const testContent = "Test file content";
  const testPath = "/tmp/bun-ios-test.txt";
  
  try {
    await Bun.write(testPath, testContent);
    const read = await Bun.file(testPath).text();
    console.log("File write/read:", read === testContent ? "OK" : "FAILED");
  } catch (error) {
    console.log("File error:", error.message);
  }
}

// Run async tests
await testFetch();
await testFiles();

console.log("\n✅ All basic tests completed!");
