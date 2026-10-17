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
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <fcntl.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

static void killSelf() {
    _exit(0);
}

static bool checkTracerPidSelf() {
    FILE* f = fopen(OBFUSCATE("/proc/self/status"), "r");
    if (!f) return false;
    char line[256];
    bool traced = false;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, OBFUSCATE("TracerPid:"), 10) == 0) {
            int pid = atoi(line + 10);
            if (pid != 0) traced = true;
            break;
        }
    }
    fclose(f);
    return traced;
}

static bool checkPtrace() {
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0) {
        return true; 
    }
    return false;
}

static bool checkFridaMaps() {
    FILE* f = fopen(OBFUSCATE("/proc/self/maps"), "r");
    if (!f) return false;
    char line[512];
    bool found = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, OBFUSCATE("frida")) ||
            strstr(line, OBFUSCATE("gum-js")) ||
            strstr(line, OBFUSCATE("gadget")) ||
            strstr(line, OBFUSCATE("linjector")) ||
            strstr(line, OBFUSCATE("gdbserver"))) {
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

// ACE Thread Freezer
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
                if (strstr(comm, OBFUSCATE("ACE")) || 
                    strstr(comm, OBFUSCATE("terrors")) || 
                    strstr(comm, OBFUSCATE("TP"))) {
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
    sleep(3); 
    while (true) {
        if (checkTracerPidSelf()) { killSelf(); }
        if (checkPtrace())        { killSelf(); }
        if (checkFridaMaps())     { killSelf(); }
        
        suspendACEThreads(); 
        sleep(2); 
    }
    return nullptr;
}

__attribute__((constructor))
void guard_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}thread, NULL);
}
