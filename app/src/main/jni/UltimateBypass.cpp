#include <jni.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

static void nukeOfflineBan() {
    const char* paths[] = {
        "/data/data/com.dts.freefiremax/files",
        "/data/data/com.dts.freefiremax/shared_prefs"
    };
    for (int i = 0; i < 2; i++) {
        DIR* d = opendir(paths[i]);
        if (d) {
            struct dirent* e;
            while ((e = readdir(d)) != NULL) {
                if (strstr(e->d_name, OBFUSCATE("ban")) || 
                    strstr(e->d_name, OBFUSCATE("config")) || 
                    strstr(e->d_name, OBFUSCATE("device"))) {
                    char full_path[256];
                    snprintf(full_path, sizeof(full_path), "%s/%s", paths[i], e->d_name);
                    unlink(full_path);
                }
            }
            closedir(d);
        }
    }
}

static void suspendACEThreads() {
    DIR* d = opendir(OBFUSCATE("/proc/self/task"));
    if (!d) return;
    struct dirent* e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
        char path[256];
        snprintf(path, sizeof(path), OBFUSCATE("/proc/self/task/%s/comm"), e->d_name);
        FILE* f = fopen(path, "r");
        if (f) {
            char comm[64] = {0};
            if (fgets(comm, sizeof(comm), f)) {
                if (strstr(comm, OBFUSCATE("ACE")) || strstr(comm, OBFUSCATE("terrors")) || strstr(comm, OBFUSCATE("TP"))) {
                    int tid = atoi(e->d_name);
                    syscall(SYS_tgkill, getpid(), tid, SIGSTOP); 
                }
            }
            fclose(f);
        }
    }
    closedir(d);
}

static void* guard_thread(void*) {
    nukeOfflineBan();
    while (true) {
        suspendACEThreads();
        sleep(1); 
    }
    return nullptr;
}

__attribute__((constructor))
void bypass_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}
