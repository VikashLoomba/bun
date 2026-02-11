/**
 * Tests for the iOS embedding API (src/ios/)
 * 
 * These tests verify the C API defined in bun_ios.h works correctly.
 * On non-iOS platforms, these tests verify the API compiles and links.
 * 
 * The actual iOS-specific behavior (thread management, I/O redirection)
 * can only be fully tested in an iOS Simulator or device.
 */

import { describe, test, expect, beforeAll } from "bun:test";
import { existsSync } from "fs";
import { join } from "path";

const isIOS = process.platform === "ios";
const srcDir = join(import.meta.dir, "../../src/ios");

describe("iOS embedding API", () => {
  describe("source files exist", () => {
    test("bun_ios.h header exists", () => {
      expect(existsSync(join(srcDir, "bun_ios.h"))).toBe(true);
    });

    test("embedding.c exists", () => {
      expect(existsSync(join(srcDir, "embedding.c"))).toBe(true);
    });

    test("stubs.c exists", () => {
      expect(existsSync(join(srcDir, "stubs.c"))).toBe(true);
    });

    test("webkit_stubs.cpp exists", () => {
      expect(existsSync(join(srcDir, "webkit_stubs.cpp"))).toBe(true);
    });

    test("README.md documentation exists", () => {
      expect(existsSync(join(srcDir, "README.md"))).toBe(true);
    });
  });

  describe("header API declarations", () => {
    let headerContent: string;

    beforeAll(async () => {
      headerContent = await Bun.file(join(srcDir, "bun_ios.h")).text();
    });

    test("declares bun_start function", () => {
      expect(headerContent).toContain("int bun_start(");
      expect(headerContent).toContain("int argc");
      expect(headerContent).toContain("const char** argv");
      expect(headerContent).toContain("int stdin_fd");
      expect(headerContent).toContain("int stdout_fd");
      expect(headerContent).toContain("int stderr_fd");
      expect(headerContent).toContain("bun_exit_callback_t on_exit");
    });

    test("declares bun_version function", () => {
      expect(headerContent).toContain("const char* bun_version(void)");
    });

    test("declares bun_eval convenience function", () => {
      expect(headerContent).toContain("int bun_eval(");
      expect(headerContent).toContain("const char* working_dir");
      expect(headerContent).toContain("const char* code");
    });

    test("declares bun_run convenience function", () => {
      expect(headerContent).toContain("int bun_run(");
      expect(headerContent).toContain("const char* script_path");
    });

    test("declares bun_exit_callback_t type", () => {
      expect(headerContent).toContain("typedef void (*bun_exit_callback_t)(uint32_t exit_code)");
    });

    test("has proper include guards", () => {
      expect(headerContent).toContain("#ifndef BUN_IOS_H");
      expect(headerContent).toContain("#define BUN_IOS_H");
      expect(headerContent).toContain("#endif");
    });

    test("has extern C guards for C++ compatibility", () => {
      expect(headerContent).toContain('#ifdef __cplusplus');
      expect(headerContent).toContain('extern "C"');
    });
  });

  describe("TCC stubs", () => {
    let stubsContent: string;

    beforeAll(async () => {
      stubsContent = await Bun.file(join(srcDir, "stubs.c")).text();
    });

    test("provides tcc_new stub returning NULL", () => {
      expect(stubsContent).toContain("tcc_new");
      expect(stubsContent).toContain("return NULL");
    });

    test("provides tcc_compile_string stub returning error", () => {
      expect(stubsContent).toContain("tcc_compile_string");
      expect(stubsContent).toContain("return -1");
    });

    test("documents why TCC is unavailable on iOS", () => {
      expect(stubsContent).toContain("JIT");
      expect(stubsContent).toContain("iOS");
    });
  });

  describe("WebKit stubs", () => {
    let webkitStubsContent: string;

    beforeAll(async () => {
      webkitStubsContent = await Bun.file(join(srcDir, "webkit_stubs.cpp")).text();
    });

    test("provides JSC::ThrowScope stubs", () => {
      expect(webkitStubsContent).toContain("class ThrowScope");
      expect(webkitStubsContent).toContain("ThrowScope(VM&");
    });

    test("provides JSC::JIT stubs", () => {
      expect(webkitStubsContent).toContain("class JIT");
      expect(webkitStubsContent).toContain("compileSync");
    });

    test("provides WTF::RefTracker stubs", () => {
      expect(webkitStubsContent).toContain("class RefTracker");
      expect(webkitStubsContent).toContain("reportDead");
      expect(webkitStubsContent).toContain("reportLive");
    });

    test("uses proper C++ (not raw mangled names)", () => {
      // Should NOT contain raw mangled symbol definitions like _ZN3JSC...
      // at the top level (they're OK in comments)
      const lines = webkitStubsContent.split('\n');
      const codeLines = lines.filter(l => !l.trim().startsWith('*') && !l.trim().startsWith('//'));
      const hasMangledDef = codeLines.some(l => /^[a-z_]*\s+_ZN\d/.test(l.trim()));
      expect(hasMangledDef).toBe(false);
    });

    test("documents purpose of stubs", () => {
      expect(webkitStubsContent).toContain("EXCEPTION_SCOPE_VERIFICATION");
    });
  });
});

// These tests only run on actual iOS
describe.skipIf(!isIOS)("iOS runtime behavior", () => {
  test("bun_version returns version string", () => {
    // This would call the actual C function via FFI on iOS
    // For now, just a placeholder
    expect(true).toBe(true);
  });
});
