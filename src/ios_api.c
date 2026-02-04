#include "ios_api.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

extern void bun_main(void);
extern void (*bun_ios_exit_callback)(uint32_t code);
int bun_ios_argc = 0;
char** bun_ios_argv = NULL;

struct bun_thread_args {
    int argc;
    char** argv;
    void (*on_exit)(uint32_t);
};

static void* bun_thread_entry(void* arg) {
    struct bun_thread_args* args = (struct bun_thread_args*)arg;

    bun_ios_exit_callback = args->on_exit;
    bun_ios_argc = args->argc;
    bun_ios_argv = args->argv;

    bun_main();

    if (args->on_exit) args->on_exit(0);

    for (int i = 0; i < args->argc; i++) free(args->argv[i]);
    free(args->argv);
    free(args);
    return NULL;
}

int bun_main_thread(int argc, char** argv, void (*on_exit)(uint32_t)) {
    struct bun_thread_args* args = malloc(sizeof(*args));
    if (!args) return -1;

    args->argc = argc;
    args->argv = malloc(sizeof(char*) * (argc + 1));
    for (int i = 0; i < argc; i++) args->argv[i] = strdup(argv[i]);
    args->argv[argc] = NULL;
    args->on_exit = on_exit;

    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, 8 * 1024 * 1024);

    int ret = pthread_create(&thread, &attr, bun_thread_entry, args);
    pthread_attr_destroy(&attr);
    return ret;
}

int bun_eval_async(const char* working_dir, const char* code, void (*on_exit)(uint32_t)) {
    struct bun_thread_args* args = malloc(sizeof(*args));
    if (!args) return -1;

    args->argc = 3;
    args->argv = malloc(sizeof(char*) * 4);
    args->argv[0] = strdup(working_dir);
    args->argv[1] = strdup("-e");
    args->argv[2] = strdup(code);
    args->argv[3] = NULL;
    args->on_exit = on_exit;

    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, 8 * 1024 * 1024);

    int ret = pthread_create(&thread, &attr, bun_thread_entry, args);
    pthread_attr_destroy(&attr);
    return ret;
}
