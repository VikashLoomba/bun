#ifndef BUN_IOS_API_H
#define BUN_IOS_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bun_exit_callback_t)(uint32_t code);

/// Run Bun with the given arguments on a new thread.
/// argv[0] should be a path to use as the "executable" location.
/// Returns 0 on success (thread started), non-zero on failure.
int bun_main_thread(int argc, char** argv, bun_exit_callback_t on_exit);

/// Convenience: evaluate JS code on a new thread.
int bun_eval_async(const char* working_dir, const char* code, bun_exit_callback_t on_exit);

/// Run Bun with redirected I/O.
/// - stdin_fd: file descriptor Bun reads from (or -1 to keep default)
/// - stdout_fd: file descriptor Bun writes to (or -1 to keep default)
/// - stderr_fd: file descriptor for stderr (or -1 to use stdout_fd, or -2 to keep default)
/// Returns 0 on success.
int bun_main_with_io(int argc, char** argv, 
                     int stdin_fd, int stdout_fd, int stderr_fd,
                     bun_exit_callback_t on_exit);

#ifdef __cplusplus
}
#endif

#endif // BUN_IOS_API_H
