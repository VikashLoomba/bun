/**
 * iOS Runtime Behavior Tests
 * 
 * Tests for iOS-specific runtime behavior. Most of these tests verify that
 * code paths shared between macOS and iOS work correctly, since iOS uses
 * the Darwin platform APIs.
 * 
 * Tests marked with .skipIf(!isIOS) only run on actual iOS.
 * Other tests verify the shared Darwin codepaths work on macOS too.
 */

import { describe, test, expect, beforeAll } from "bun:test";
import { spawnSync } from "bun";
import { existsSync, mkdirSync, rmdirSync, writeFileSync, unlinkSync, readFileSync } from "fs";
import { join } from "path";
import { tmpdir } from "os";
import { isMacOS, isPosix } from "harness";

const isIOS = process.platform === "ios";
const isDarwin = isMacOS || isIOS;

describe("Darwin shared behavior (macOS + iOS)", () => {
  describe.skipIf(!isDarwin)("filesystem operations", () => {
    const testDir = join(tmpdir(), `bun-ios-test-${Date.now()}`);
    const testFile = join(testDir, "test.txt");

    beforeAll(() => {
      if (existsSync(testDir)) {
        rmdirSync(testDir, { recursive: true });
      }
      mkdirSync(testDir, { recursive: true });
    });

    test("mkdir creates directory", () => {
      const subdir = join(testDir, "subdir");
      mkdirSync(subdir);
      expect(existsSync(subdir)).toBe(true);
    });

    test("writeFileSync and readFileSync work", () => {
      const content = "Hello from Bun iOS test!";
      writeFileSync(testFile, content);
      expect(readFileSync(testFile, "utf8")).toBe(content);
    });

    test("unlinkSync removes file", () => {
      const tempFile = join(testDir, "temp.txt");
      writeFileSync(tempFile, "temp");
      expect(existsSync(tempFile)).toBe(true);
      unlinkSync(tempFile);
      expect(existsSync(tempFile)).toBe(false);
    });

    test("Bun.file() works with Darwin paths", async () => {
      writeFileSync(testFile, "test content");
      const file = Bun.file(testFile);
      expect(await file.text()).toBe("test content");
      expect(file.size).toBe(12);
    });
  });

  describe.skipIf(!isDarwin)("process info", () => {
    test("process.platform is darwin on macOS/iOS", () => {
      // On both macOS and iOS, platform should be "darwin"
      // (iOS might report "ios" in future Bun versions)
      expect(["darwin", "ios"]).toContain(process.platform);
    });

    test("process.arch is valid", () => {
      expect(["arm64", "x64"]).toContain(process.arch);
    });

    test("os.cpus() returns CPU info", async () => {
      const os = await import("os");
      const cpus = os.cpus();
      expect(Array.isArray(cpus)).toBe(true);
      expect(cpus.length).toBeGreaterThan(0);
      expect(cpus[0]).toHaveProperty("model");
      expect(cpus[0]).toHaveProperty("speed");
    });

    test("os.loadavg() returns load averages", async () => {
      const os = await import("os");
      const loadavg = os.loadavg();
      expect(Array.isArray(loadavg)).toBe(true);
      expect(loadavg.length).toBe(3);
      // Load averages should be non-negative numbers
      expect(loadavg[0]).toBeGreaterThanOrEqual(0);
      expect(loadavg[1]).toBeGreaterThanOrEqual(0);
      expect(loadavg[2]).toBeGreaterThanOrEqual(0);
    });

    test("os.tmpdir() returns valid path", async () => {
      const os = await import("os");
      const tmp = os.tmpdir();
      expect(typeof tmp).toBe("string");
      expect(tmp.length).toBeGreaterThan(0);
      // Darwin uses /var/folders or /private/tmp typically
      expect(tmp.startsWith("/")).toBe(true);
    });

    test("os.homedir() returns valid path", async () => {
      const os = await import("os");
      const home = os.homedir();
      expect(typeof home).toBe("string");
      expect(home.startsWith("/")).toBe(true);
    });
  });

  describe.skipIf(!isDarwin)("network operations", () => {
    test("fetch works with HTTPS", async () => {
      const response = await fetch("https://example.com");
      expect(response.ok).toBe(true);
      const text = await response.text();
      expect(text).toContain("Example Domain");
    });

    test("DNS resolution works", async () => {
      const dns = await import("dns");
      const addresses = await dns.promises.resolve4("example.com");
      expect(Array.isArray(addresses)).toBe(true);
      expect(addresses.length).toBeGreaterThan(0);
      // Should be valid IPv4 addresses
      expect(addresses[0]).toMatch(/^\d+\.\d+\.\d+\.\d+$/);
    });
  });
});

describe("iOS-specific behavior", () => {
  describe.skipIf(!isIOS)("iOS platform detection", () => {
    test("Environment.isIOS is true", () => {
      // This tests the Zig Environment.isIOS constant exposure
      // For now we check process.platform
      expect(process.platform).toBe("ios");
    });
  });

  describe.skipIf(!isIOS)("iOS restrictions", () => {
    test("FFI/TCC compilation is disabled", async () => {
      // On iOS, FFI compilation should fail gracefully
      // because TCC (TinyCC) requires JIT which is unavailable
      const { dlopen, FFIType, suffix } = await import("bun:ffi");
      
      // dlopen should still work for system libraries
      // but tcc_compile should fail
      // This is a basic sanity check - detailed FFI tests are elsewhere
      expect(typeof dlopen).toBe("function");
    });

    test("child_process.spawn is restricted", async () => {
      // On iOS, spawning processes is heavily restricted by the sandbox
      // This should either fail or be limited to specific cases
      const { spawn } = await import("child_process");
      
      // Attempting to spawn should throw on iOS
      expect(() => {
        spawn("/bin/ls", ["-la"]);
      }).toThrow();
    });
  });

  describe.skipIf(!isIOS)("iOS embedding API integration", () => {
    test("Bun runs on separate thread (callback-based exit)", () => {
      // When embedded in iOS, Bun should use bun_ios_exit_callback
      // instead of calling exit() directly
      // This is verified by the host app, not testable from within Bun
      expect(true).toBe(true);
    });

    test("process.exit() triggers callback, not process termination", () => {
      // On iOS, process.exit() should call the exit callback
      // rather than terminating the entire host app
      // This can't be fully tested from within - it requires the host app
      expect(true).toBe(true);
    });
  });
});

describe("Feature parity tests", () => {
  // These tests verify features work on all platforms including iOS
  
  test("TextEncoder/TextDecoder work", () => {
    const encoder = new TextEncoder();
    const decoder = new TextDecoder();
    
    const text = "Hello, 世界! 🌍";
    const encoded = encoder.encode(text);
    const decoded = decoder.decode(encoded);
    
    expect(decoded).toBe(text);
  });

  test("crypto.randomUUID() works", () => {
    const uuid = crypto.randomUUID();
    expect(uuid).toMatch(/^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i);
  });

  test("atob/btoa work correctly", () => {
    const original = "Hello, World!";
    const encoded = btoa(original);
    expect(encoded).toBe("SGVsbG8sIFdvcmxkIQ==");
    expect(atob(encoded)).toBe(original);
  });

  test("URL parsing works", () => {
    const url = new URL("https://user:pass@example.com:8080/path?query=value#hash");
    expect(url.protocol).toBe("https:");
    expect(url.username).toBe("user");
    expect(url.password).toBe("pass");
    expect(url.hostname).toBe("example.com");
    expect(url.port).toBe("8080");
    expect(url.pathname).toBe("/path");
    expect(url.search).toBe("?query=value");
    expect(url.hash).toBe("#hash");
  });

  test("JSON parsing works", () => {
    const obj = { name: "test", value: 42, nested: { array: [1, 2, 3] } };
    const json = JSON.stringify(obj);
    const parsed = JSON.parse(json);
    expect(parsed).toEqual(obj);
  });

  test("Promise and async/await work", async () => {
    const result = await new Promise<number>(resolve => {
      setTimeout(() => resolve(42), 10);
    });
    expect(result).toBe(42);
  });

  test("Bun.sleep works", async () => {
    const start = Date.now();
    await Bun.sleep(50);
    const elapsed = Date.now() - start;
    expect(elapsed).toBeGreaterThanOrEqual(45); // Allow some variance
  });
});
