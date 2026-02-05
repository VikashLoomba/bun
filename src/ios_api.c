#include "ios_api.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <util.h>  // for forkpty on Darwin
#include <termios.h>
#include <fcntl.h>

// External Bun entry points
extern void bun_main(void);
extern void (*bun_ios_exit_callback)(uint32_t code);

// These are read by Bun to get command-line arguments on iOS
int bun_ios_argc = 0;
char** bun_ios_argv = NULL;

// Global state for pty mode
static int g_pty_master = -1;
static bun_output_callback_t g_output_callback = NULL;

struct bun_thread_args {
    int argc;
    char** argv;
    bun_exit_callback_t on_exit;
    int pty_slave;  // -1 if not using pty
};

static void* bun_thread_entry(void* arg) {
    struct bun_thread_args* args = (struct bun_thread_args*)arg;

    // Set up pty if provided
    if (args->pty_slave >= 0) {
        dup2(args->pty_slave, STDIN_FILENO);
        dup2(args->pty_slave, STDOUT_FILENO);
        dup2(args->pty_slave, STDERR_FILENO);
        if (args->pty_slave > STDERR_FILENO) {
            close(args->pty_slave);
        }
    }

    bun_ios_exit_callback = args->on_exit;
    bun_ios_argc = args->argc;
    bun_ios_argv = args->argv;

    bun_main();

    // If we get here, bun_main returned without calling exit
    if (args->on_exit) args->on_exit(0);

    // Cleanup
    for (int i = 0; i < args->argc; i++) free(args->argv[i]);
    free(args->argv);
    free(args);
    return NULL;
}

static int start_bun_thread(struct bun_thread_args* args) {
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, 8 * 1024 * 1024);

    int ret = pthread_create(&thread, &attr, bun_thread_entry, args);
    pthread_attr_destroy(&attr);
    return ret;
}

int bun_main_thread(int argc, char** argv, bun_exit_callback_t on_exit) {
    struct bun_thread_args* args = malloc(sizeof(*args));
    if (!args) return -1;

    args->argc = argc;
    args->argv = malloc(sizeof(char*) * (argc + 1));
    for (int i = 0; i < argc; i++) args->argv[i] = strdup(argv[i]);
    args->argv[argc] = NULL;
    args->on_exit = on_exit;
    args->pty_slave = -1;

    return start_bun_thread(args);
}

int bun_eval_async(const char* working_dir, const char* code, bun_exit_callback_t on_exit) {
    struct bun_thread_args* args = malloc(sizeof(*args));
    if (!args) return -1;

    args->argc = 3;
    args->argv = malloc(sizeof(char*) * 4);
    args->argv[0] = strdup(working_dir);
    args->argv[1] = strdup("-e");
    args->argv[2] = strdup(code);
    args->argv[3] = NULL;
    args->on_exit = on_exit;
    args->pty_slave = -1;

    return start_bun_thread(args);
}

int bun_main_with_pty(int argc, char** argv, bun_exit_callback_t on_exit) {
    int master_fd;
    int slave_fd;
    
    // Open a new pty pair
    struct termios termp;
    struct winsize winp = {.ws_row = 24, .ws_col = 80, .ws_xpixel = 0, .ws_ypixel = 0};
    
    // Use openpty instead of forkpty since we're using threads, not fork
    if (openpty(&master_fd, &slave_fd, NULL, NULL, &winp) < 0) {
        return -1;
    }
    
    // Configure terminal
    if (tcgetattr(slave_fd, &termp) == 0) {
        // Raw mode for proper terminal handling
        cfmakeraw(&termp);
        termp.c_lflag |= ISIG;  // Keep signals
        termp.c_oflag |= OPOST | ONLCR;  // Process output
        tcsetattr(slave_fd, TCSANOW, &termp);
    }
    
    // Make master non-blocking
    int flags = fcntl(master_fd, F_GETFL, 0);
    fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);
    
    g_pty_master = master_fd;

    struct bun_thread_args* args = malloc(sizeof(*args));
    if (!args) {
        close(master_fd);
        close(slave_fd);
        return -1;
    }

    args->argc = argc;
    args->argv = malloc(sizeof(char*) * (argc + 1));
    for (int i = 0; i < argc; i++) args->argv[i] = strdup(argv[i]);
    args->argv[argc] = NULL;
    args->on_exit = on_exit;
    args->pty_slave = slave_fd;

    if (start_bun_thread(args) != 0) {
        close(master_fd);
        close(slave_fd);
        free(args->argv);
        free(args);
        g_pty_master = -1;
        return -1;
    }

    return master_fd;
}

int bun_write_stdin(const char* data, uint32_t len) {
    if (g_pty_master < 0) return -1;
    return (int)write(g_pty_master, data, len);
}

void bun_set_output_callback(bun_output_callback_t callback) {
    g_output_callback = callback;
}
