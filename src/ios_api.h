#ifndef BUN_IOS_API_H
#define BUN_IOS_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Run Bun with the given arguments on a new thread.
/// argv[0] should be a path to use as the "executable" location.
/// Returns 0 on success (thread started), non-zero on failure.
int bun_main_thread(int argc, char** argv, void (*on_exit)(uint32_t code));

/// Convenience: evaluate JS code on a new thread.
/// `working_dir` is used as cwd. `on_exit` is called when done.
int bun_eval_async(const char* working_dir, const char* code, void (*on_exit)(uint32_t code));

#ifdef __cplusplus
}
#endif

#endif // BUN_IOS_API_H
