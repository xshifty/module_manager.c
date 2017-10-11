#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>
#include <dirent.h>

#include "manager.h"

void loop_start(void (*func_ptr)())
{
    for (;;) {
        func_ptr();
        fflush(stdout);
        fflush(stderr);
    }
}

int main()
{
    DIR *mod_dir_handler;
    struct dirent *mod_dirent;
    char *modules[MAX_LOADABLE_MODULES + 1], prog_path[PATH_MAX], enabled_path[PATH_MAX];
    void *handler[MAX_LOADABLE_MODULES], (*setup[MAX_LOADABLE_MODULES])(), (*loop[MAX_LOADABLE_MODULES])();
    pthread_t jobs[MAX_LOADABLE_MODULES];
    int running[MAX_LOADABLE_MODULES], i, modules_found = 0;

    memset(prog_path, 0, PATH_MAX);
    memset(enabled_path, 0, PATH_MAX);

    if ( readlink("/proc/self/exe", prog_path, PATH_MAX) == -1 ) {
        fprintf(stderr, "[ CAN'T GET PROGRAM PATH ]\n");
        exit(1);
    }

    sprintf(enabled_path, "%s/modules-enabled", dirname(prog_path));
    mod_dir_handler = opendir(enabled_path);

    if (!mod_dir_handler) {
        fprintf(stderr, "[ CAN'T FIND MODULES ENABLED PATH - %s ]\n", enabled_path);
        exit(1);
    }

    i = 0;
    while ( (mod_dirent = readdir(mod_dir_handler)) != NULL && i <= MAX_LOADABLE_MODULES ) {
        if ( strstr(mod_dirent->d_name, ".so") != NULL ) {
            modules[i] = (char *) malloc(sizeof(char) * PATH_MAX);
            sprintf(modules[i], "%s/%s", enabled_path, mod_dirent->d_name);
            printf("[ FOUND %s ]\n", basename(modules[i]));
            modules_found++;
            i++;
        }
    }
    closedir(mod_dir_handler);
    modules[i] = NULL;

    if (modules_found == 0) {
        printf("[ NO MODULES FOUND! EXITING! ]\n");
        exit(0);
    }

    i = 0;
    dlerror();
    while (modules[i] != NULL) {
        printf("[ LOADING %s ]\n", basename(modules[i]));
        handler[i] = dlopen(modules[i], RTLD_LAZY);

        if (dlerror()) {
            fprintf(stderr, "[ %s ]\n", dlerror());
            i++;
            continue;
        }

        setup[i] = dlsym(handler[i], "setup");
        if (dlerror()) {
            fprintf(stderr, "[ %s ]\n", dlerror());
            i++;
            continue;
        }

        loop[i] = dlsym(handler[i], "loop");
        if (dlerror()) {
            fprintf(stderr, "[ %s ]\n", dlerror());
            i++;
            continue;
        }

        printf("[ OK %s ]\n", basename(modules[i]));
        printf("[ CALLING %s@setup() ]\n", basename(modules[i]));
        setup[i]();
        i++;
    }

    for (;;) {
        i = 0;
        while (modules[i] != NULL) {
            if ( !jobs[i] || !(pthread_kill(jobs[i], 0) == 0) ) {
                printf("[ CALLING %s@loop() ]\n", basename(modules[i]));
                running[i] = pthread_create(&jobs[i], NULL, (void*)loop_start, loop[i]);
                if (running[i]) {
                    fprintf(stderr, "[ CAN'T START MODULE %s ]\n", modules[i]);
                }
            }

            i++;
        }

        usleep(1000);
    }

    return 0;
}
