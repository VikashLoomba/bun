#ifndef BUN_IOS_API_H
#define BUN_IOS_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Callback types
typedef void (*bun_exit_callback_t)(uint32_t code);
typedef void (*bun_output_callback_t)(const char* data, uint32_t len);

/// Run Bun with the given arguments on a new thread.
/// argv[0] should be a path to use as the "executable" location.
/// Returns 0 on success (thread started), non-zero on failure.
int bun_main_thread(int argc, char** argv, bun_exit_callback_t on_exit);

/// Convenience: evaluate JS code on a new thread.
/// `working_dir` is used as cwd. `on_exit` is called when done.
int bun_eval_async(const char* working_dir, const char* code, bun_exit_callback_t on_exit);

/// Run Bun with pty-based I/O for terminal integration.
/// Returns the pty master fd on success, or -1 on failure.
/// The caller should:
///   - Read from the returned fd to get Bun's output
///   - Write to the returned fd to send input to Bun
/// `on_exit` is called when Bun terminates.
int bun_main_with_pty(int argc, char** argv, bun_exit_callback_t on_exit);

/// Send input to Bun's stdin (when using pty mode).
/// Returns number of bytes written, or -1 on error.
int bun_write_stdin(const char* data, uint32_t len);

/// Set callback for Bun's stdout/stderr output (alternative to pty).
/// The callback is called from Bun's thread whenever output is produced.
void bun_set_output_callback(bun_output_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // BUN_IOS_API_H
