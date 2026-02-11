/**
 * iOS Platform Stubs
 * 
 * This file provides stub implementations for features that are unavailable
 * on iOS due to platform restrictions:
 * 
 * 1. TinyCC (TCC) - Requires JIT/dynamic code generation
 * 2. FFI JIT callbacks - Requires executable memory allocation
 * 
 * These stubs allow Bun to compile and link on iOS while gracefully
 * disabling the affected features at runtime.
 */

#include <stddef.h>

/*
 * TinyCC Stubs
 * 
 * TCC is used by Bun's FFI implementation to compile C code at runtime.
 * This is not possible on iOS because:
 * - iOS prohibits JIT compilation (no W+X memory pages)
 * - Apps cannot execute dynamically generated code
 * 
 * When FFI compilation is attempted on iOS, these stubs return error codes
 * that cause Bun to report "FFI compilation not available on iOS".
 */

typedef struct TCCState TCCState;

TCCState* tcc_new(void) { 
    return NULL;  /* Indicates TCC is unavailable */
}

void tcc_delete(TCCState* s) { 
    (void)s; 
}

int tcc_set_output_type(TCCState* s, int output_type) { 
    (void)s; 
    (void)output_type; 
    return -1;  /* Error */
}

int tcc_compile_string(TCCState* s, const char* buf) { 
    (void)s; 
    (void)buf; 
    return -1;  /* Error */
}

int tcc_add_file(TCCState* s, const char* filename) { 
    (void)s; 
    (void)filename; 
    return -1;  /* Error */
}

int tcc_relocate(TCCState* s) { 
    (void)s; 
    return -1;  /* Error */
}

void* tcc_get_symbol(TCCState* s, const char* name) { 
    (void)s; 
    (void)name; 
    return NULL;  /* Symbol not found */
}

int tcc_add_symbol(TCCState* s, const char* name, const void* val) { 
    (void)s; 
    (void)name; 
    (void)val; 
    return -1;  /* Error */
}

void tcc_define_symbol(TCCState* s, const char* sym, const char* value) { 
    (void)s; 
    (void)sym; 
    (void)value; 
}

int tcc_add_library(TCCState* s, const char* libraryname) { 
    (void)s; 
    (void)libraryname; 
    return -1;  /* Error */
}

int tcc_add_library_path(TCCState* s, const char* pathname) { 
    (void)s; 
    (void)pathname; 
    return -1;  /* Error */
}

int tcc_add_sysinclude_path(TCCState* s, const char* pathname) { 
    (void)s; 
    (void)pathname; 
    return -1;  /* Error */
}

void tcc_set_error_func(TCCState* s, void* error_opaque, 
                        void (*error_func)(void*, const char*)) { 
    (void)s; 
    (void)error_opaque; 
    (void)error_func; 
}

void tcc_set_options(TCCState* s, const char* str) { 
    (void)s; 
    (void)str; 
}

/*
 * ARM64 Stack Pointer Helper
 * 
 * This function returns the current stack pointer. It's used by WebKit's
 * conservative garbage collector to scan the stack for GC roots.
 * 
 * Normally this is provided by libWTF.a, but it may be missing from
 * iOS builds that use a minimal WebKit configuration.
 */
#if defined(__aarch64__) || defined(__arm64__)
__asm__(
    ".text\n"
    ".balign 16\n"
    ".globl _currentStackPointer\n"
    "_currentStackPointer:\n"
    "    mov x0, sp\n"
    "    ret\n"
    ".previous\n"
);
#endif
