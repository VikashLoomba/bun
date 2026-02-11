/**
 * Bun iOS Embedding API
 * 
 * This header provides the public API for embedding Bun in iOS applications.
 * Bun runs on a separate thread with redirected I/O, allowing the host app
 * to communicate via file descriptors (typically pipes).
 * 
 * Example usage:
 * 
 *     #include "bun_ios.h"
 *     
 *     void on_exit(uint32_t code) {
 *         printf("Bun exited with code %u\n", code);
 *     }
 *     
 *     int main() {
 *         const char* argv[] = {"bun", "run", "script.js", NULL};
 *         int result = bun_start(3, argv, -1, -1, -1, on_exit);
 *         // ...
 *     }
 */

#ifndef BUN_IOS_H
#define BUN_IOS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Callback invoked when Bun exits.
 * 
 * @param exit_code The exit code from the script (0 = success)
 * 
 * Note: This callback is invoked on the Bun thread, not the main thread.
 * Use dispatch_async() if you need to update UI.
 */
typedef void (*bun_exit_callback_t)(uint32_t exit_code);

/**
 * Start Bun with the given arguments and I/O configuration.
 * 
 * Bun runs on a dedicated thread with an 8MB stack. The thread is detached,
 * so you don't need to join it. Use the exit callback to know when it finishes.
 * 
 * @param argc       Number of arguments (including argv[0])
 * @param argv       Argument array. argv[0] should be "bun" or a path.
 *                   The array is copied, so it can be freed after this call.
 * @param stdin_fd   File descriptor for stdin:
 *                   - >= 0: Use this fd (e.g., pipe read end)
 *                   - -1: Use /dev/null
 *                   - -2: Keep inherited stdin
 * @param stdout_fd  File descriptor for stdout:
 *                   - >= 0: Use this fd (e.g., pipe write end)
 *                   - -1: Use /dev/null
 *                   - -2: Keep inherited stdout
 * @param stderr_fd  File descriptor for stderr:
 *                   - >= 0: Use this fd
 *                   - -1: Same as stdout_fd
 *                   - -2: Keep inherited stderr
 * @param on_exit    Callback invoked when Bun exits (can be NULL)
 * 
 * @return 0 on success (thread started), -1 on failure
 */
int bun_start(
    int argc,
    const char** argv,
    int stdin_fd,
    int stdout_fd,
    int stderr_fd,
    bun_exit_callback_t on_exit
);

/**
 * Get the Bun version string.
 * 
 * @return Version string (e.g., "1.2.3"). Do not free.
 */
const char* bun_version(void);

/**
 * Convenience function to evaluate JavaScript code.
 * 
 * Equivalent to: bun_start(3, {"bun", "-e", code}, ...)
 * 
 * @param working_dir Working directory for the script
 * @param code        JavaScript code to evaluate
 * @param stdin_fd    stdin file descriptor (see bun_start)
 * @param stdout_fd   stdout file descriptor (see bun_start)
 * @param stderr_fd   stderr file descriptor (see bun_start)
 * @param on_exit     Exit callback (can be NULL)
 * 
 * @return 0 on success, -1 on failure
 */
int bun_eval(
    const char* working_dir,
    const char* code,
    int stdin_fd,
    int stdout_fd,
    int stderr_fd,
    bun_exit_callback_t on_exit
);

/**
 * Run a script file.
 * 
 * Equivalent to: bun_start(3, {"bun", "run", script_path}, ...)
 * 
 * @param script_path Path to the JavaScript/TypeScript file
 * @param stdin_fd    stdin file descriptor (see bun_start)
 * @param stdout_fd   stdout file descriptor (see bun_start)  
 * @param stderr_fd   stderr file descriptor (see bun_start)
 * @param on_exit     Exit callback (can be NULL)
 * 
 * @return 0 on success, -1 on failure
 */
int bun_run(
    const char* script_path,
    int stdin_fd,
    int stdout_fd,
    int stderr_fd,
    bun_exit_callback_t on_exit
);

#ifdef __cplusplus
}
#endif

#endif /* BUN_IOS_H */
