#include <stddef.h>

// TCC (TinyCC) stubs for iOS — TCC requires JIT which is unavailable on iOS

typedef struct TCCState TCCState;

TCCState* tcc_new(void) { return NULL; }
void tcc_delete(TCCState* s) { (void)s; }
int tcc_set_output_type(TCCState* s, int output_type) { (void)s; (void)output_type; return -1; }
int tcc_compile_string(TCCState* s, const char* buf) { (void)s; (void)buf; return -1; }
int tcc_add_file(TCCState* s, const char* filename) { (void)s; (void)filename; return -1; }
int tcc_relocate(TCCState* s) { (void)s; return -1; }
void* tcc_get_symbol(TCCState* s, const char* name) { (void)s; (void)name; return NULL; }
int tcc_add_symbol(TCCState* s, const char* name, const void* val) { (void)s; (void)name; (void)val; return -1; }
void tcc_define_symbol(TCCState* s, const char* sym, const char* value) { (void)s; (void)sym; (void)value; }
int tcc_add_library(TCCState* s, const char* libraryname) { (void)s; (void)libraryname; return -1; }
int tcc_add_library_path(TCCState* s, const char* pathname) { (void)s; (void)pathname; return -1; }
int tcc_add_sysinclude_path(TCCState* s, const char* pathname) { (void)s; (void)pathname; return -1; }
void tcc_set_error_func(TCCState* s, void* error_opaque, void (*error_func)(void*, const char*)) { (void)s; (void)error_opaque; (void)error_func; }
void tcc_set_options(TCCState* s, const char* str) { (void)s; (void)str; }

// currentStackPointer — ARM64 assembly, normally in libWTF.a
// Missing from our older WebKit build
__asm__(
    ".text\n"
    ".balign 16\n"
    ".globl _currentStackPointer\n"
    "_currentStackPointer:\n"
    "mov x0, sp\n"
    "ret\n"
    ".previous\n"
);
