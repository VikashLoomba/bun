/**
 * WebKit/JSC Stubs for iOS
 * 
 * Provides stub implementations for symbols missing from the iOS JSC build.
 * 
 * Why these are needed:
 * - The JSC library was built with ENABLE(EXCEPTION_SCOPE_VERIFICATION)
 * - JIT-related symbols are referenced even when JIT is disabled
 * - Some debug/assertion code paths need these symbols
 * 
 * Proper fix: Rebuild JavaScriptCore with correct iOS configuration.
 */

#include <cstddef>
#include <cstdint>

#define NOINLINE __attribute__((noinline))

// =============================================================================
// WTF (Web Template Framework) stubs
// =============================================================================

namespace WTF {

class StringImpl;

namespace Detail {
    int wtfStringCopyCount = 0;
}

class RefTracker {
public:
    NOINLINE static void reportDead(void*);
    NOINLINE static void reportLive(void*);
};

void RefTracker::reportDead(void*) {}
void RefTracker::reportLive(void*) {}

class AtomStringImpl {
public:
    NOINLINE static bool isInAtomStringTable(StringImpl*);
};

bool AtomStringImpl::isInAtomStringTable(StringImpl*) { return true; }

template<typename E> struct ConstexprOptionSet {
    constexpr ConstexprOptionSet() = default;
};

} // namespace WTF

// =============================================================================
// JSC (JavaScriptCore) stubs
// =============================================================================

namespace JSC {

class VM;
class JSGlobalObject;
class Exception;
class CodeBlock;
class HandleNode;

// JSValue - 64-bit encoded value
class JSValue {
    int64_t u;
public:
    JSValue() : u(0) {}
};

// Exception location tracking
struct ExceptionEventLocation {
    void* stackPosition;
    const char* functionName;
    const char* file;
    unsigned line;
};

// Base exception scope
class ExceptionScope {
protected:
    VM& m_vm;
public:
    ExceptionScope(VM& vm) : m_vm(vm) {}
    NOINLINE static const char* unexpectedExceptionMessage();
};

const char* ExceptionScope::unexpectedExceptionMessage() {
    return "Unexpected exception (iOS stub)";
}

// ThrowScope - tracks where exceptions may be thrown
class ThrowScope : public ExceptionScope {
public:
    NOINLINE ThrowScope(VM& vm, ExceptionEventLocation);
    NOINLINE ~ThrowScope();
    
    NOINLINE Exception* throwException(JSGlobalObject*, Exception*);
    NOINLINE Exception* throwException(JSGlobalObject*, JSValue);
};

ThrowScope::ThrowScope(VM& vm, ExceptionEventLocation) : ExceptionScope(vm) {}
ThrowScope::~ThrowScope() {}
Exception* ThrowScope::throwException(JSGlobalObject*, Exception*) { __builtin_trap(); }
Exception* ThrowScope::throwException(JSGlobalObject*, JSValue) { __builtin_trap(); }

// TopExceptionScope
class TopExceptionScope : public ExceptionScope {
public:
    NOINLINE TopExceptionScope(VM& vm, ExceptionEventLocation);
    NOINLINE ~TopExceptionScope();
};

TopExceptionScope::TopExceptionScope(VM& vm, ExceptionEventLocation) : ExceptionScope(vm) {}
TopExceptionScope::~TopExceptionScope() {}

// ObjectInitializationScope
class ObjectInitializationScope {
    VM& m_vm;
public:
    NOINLINE ObjectInitializationScope(VM& vm);
    NOINLINE ~ObjectInitializationScope();
};

ObjectInitializationScope::ObjectInitializationScope(VM& vm) : m_vm(vm) {}
ObjectInitializationScope::~ObjectInitializationScope() {}

// AssertNoGC
class AssertNoGC {
public:
    static unsigned s_scopeReentryCount;
};
unsigned AssertNoGC::s_scopeReentryCount = 0;

// DFG::DoesGCCheck
namespace DFG {
class DoesGCCheck {
public:
    NOINLINE static void verifyCanGC(VM&);
};

void DoesGCCheck::verifyCanGC(VM&) {}
}

// HandleSet
class HandleSet {
public:
    NOINLINE static bool isLiveNode(HandleNode*);
};

bool HandleSet::isLiveNode(HandleNode*) { return true; }

// MarkedBlock
class MarkedBlock {
public:
    NOINLINE void assertMarksNotStale();
};

void MarkedBlock::assertMarksNotStale() {}

// RegExpObject
class RegExpObject {
public:
    NOINLINE void finishCreation(VM&);
};

void RegExpObject::finishCreation(VM&) {}

// StrongRefTracker
class StrongRefTracker {
public:
    NOINLINE static void* refTrackerSingleton();
};

void* StrongRefTracker::refTrackerSingleton() {
    static char dummy[64];
    return dummy;
}

// =============================================================================
// JIT stubs
// =============================================================================

enum class JITCompilationEffort : uint8_t {};

class JIT {
public:
    NOINLINE static void* compileSync(VM&, CodeBlock*, JITCompilationEffort);
    NOINLINE static double totalCompileTime();
};

void* JIT::compileSync(VM&, CodeBlock*, JITCompilationEffort) { return nullptr; }
double JIT::totalCompileTime() { return 0.0; }

// performJITMemcpy
enum class RepatchingFlag : uint8_t {};

template<WTF::ConstexprOptionSet<RepatchingFlag>>
NOINLINE void* performJITMemcpy(void* dst, const void* src, size_t n) {
    return __builtin_memcpy(dst, src, n);
}

template void* performJITMemcpy<WTF::ConstexprOptionSet<RepatchingFlag>{}>(void*, const void*, size_t);

} // namespace JSC

// =============================================================================
// Bun-specific stubs
// =============================================================================

extern "C" {
const char* Bun__CallFrame__describeFrame(void*) {
    return "<frame>";
}
}
