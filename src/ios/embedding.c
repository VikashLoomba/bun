/**
 * Bun iOS Embedding Implementation
 * 
 * This file implements the public embedding API defined in bun_ios.h.
 * It handles thread creation, I/O redirection, and argument passing.
 */

#include "bun_ios.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* Bun's main entry point (exported from main.zig) */
extern void bun_main(void);

/* Exit callback storage (read by Global.zig) */
extern void (*bun_ios_exit_callback)(uint32_t code);

/* Argument storage (read by bun.zig) */
int bun_ios_argc = 0;
char** bun_ios_argv = NULL;

/* Thread arguments structure */
struct bun_thread_args {
    int argc;
    char** argv;
    bun_exit_callback_t on_exit;
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;
};

/* Redirect a standard file descriptor */
static void redirect_fd(int target_fd, int new_fd, int fallback_fd) {
    if (new_fd >= 0) {
        /* Use the provided fd */
        dup2(new_fd, target_fd);
        if (new_fd > STDERR_FILENO) {
            close(new_fd);
        }
    } else if (new_fd == -1) {
        if (fallback_fd >= 0) {
            /* -1 with fallback means "use fallback" (e.g., stderr -> stdout) */
            dup2(fallback_fd, target_fd);
        } else {
            /* -1 without fallback means /dev/null */
            int devnull = open("/dev/null", target_fd == STDIN_FILENO ? O_RDONLY : O_WRONLY);
            if (devnull >= 0) {
                dup2(devnull, target_fd);
                close(devnull);
            }
        }
    }
    /* -2 means keep default, do nothing */
}

/* Thread entry point */
static void* bun_thread_entry(void* arg) {
    struct bun_thread_args* args = (struct bun_thread_args*)arg;

    /* Set up I/O redirection */
    redirect_fd(STDIN_FILENO, args->stdin_fd, -1);
    redirect_fd(STDOUT_FILENO, args->stdout_fd, -1);
    
    /* stderr: -1 means same as stdout */
    if (args->stderr_fd == -1 && args->stdout_fd >= 0) {
        dup2(STDOUT_FILENO, STDERR_FILENO);
    } else {
        redirect_fd(STDERR_FILENO, args->stderr_fd, -1);
    }

    /* Set up globals for Bun */
    bun_ios_exit_callback = args->on_exit;
    bun_ios_argc = args->argc;
    bun_ios_argv = args->argv;

    /* Run Bun */
    bun_main();

    /* If we get here, Bun exited normally */
    if (args->on_exit) {
        args->on_exit(0);
    }

    /* Cleanup */
    for (int i = 0; i < args->argc; i++) {
        free(args->argv[i]);
    }
    free(args->argv);
    free(args);

    return NULL;
}

/* Create and start the Bun thread */
static int start_bun_thread(struct bun_thread_args* args) {
    pthread_t thread;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    /* Bun needs a large stack for deep recursion */
    pthread_attr_setstacksize(&attr, 8 * 1024 * 1024);

    int ret = pthread_create(&thread, &attr, bun_thread_entry, args);
    pthread_attr_destroy(&attr);
    
    return ret;
}

/* Copy argv array */
static char** copy_argv(int argc, const char** argv) {
    char** copy = malloc(sizeof(char*) * (argc + 1));
    if (!copy) return NULL;
    
    for (int i = 0; i < argc; i++) {
        copy[i] = strdup(argv[i]);
        if (!copy[i]) {
            /* Cleanup on failure */
            for (int j = 0; j < i; j++) free(copy[j]);
            free(copy);
            return NULL;
        }
    }
    copy[argc] = NULL;
    
    return copy;
}

/* Public API: bun_start */
int bun_start(
    int argc,
    const char** argv,
    int stdin_fd,
    int stdout_fd,
    int stderr_fd,
    bun_exit_callback_t on_exit
) {
    struct bun_thread_args* args = malloc(sizeof(*args));
    if (!args) return -1;

    args->argv = copy_argv(argc, argv);
    if (!args->argv) {
        free(args);
        return -1;
    }
    
    args->argc = argc;
    args->on_exit = on_exit;
    args->stdin_fd = stdin_fd;
    args->stdout_fd = stdout_fd;
    args->stderr_fd = stderr_fd;

    return start_bun_thread(args);
}

/* Public API: bun_version */
const char* bun_version(void) {
    /* Version is set at build time via -DBUN_VERSION_STRING */
#ifdef BUN_VERSION_STRING
    return BUN_VERSION_STRING;
#else
    return "1.0.0";  /* Fallback if not defined */
#endif
}

/* Public API: bun_eval */
int bun_eval(
    const char* working_dir,
    const char* code,
    int stdin_fd,
    int stdout_fd,
    int stderr_fd,
    bun_exit_callback_t on_exit
) {
    const char* argv[] = {working_dir, "-e", code};
    return bun_start(3, argv, stdin_fd, stdout_fd, stderr_fd, on_exit);
}

/* Public API: bun_run */
int bun_run(
    const char* script_path,
    int stdin_fd,
    int stdout_fd,
    int stderr_fd,
    bun_exit_callback_t on_exit
) {
    const char* argv[] = {"bun", "run", script_path};
    return bun_start(3, argv, stdin_fd, stdout_fd, stderr_fd, on_exit);
}
