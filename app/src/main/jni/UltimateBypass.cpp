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
#include <dlfcn.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Dobby/dobby.h"

// =====================================================================
// 1. DOBBY HOOKS - KERNEL SYSCALL INTERCEPTION
// =====================================================================
static int (*orig_ptrace)(int request, pid_t pid, void *addr, void *data) = nullptr;
static FILE* (*orig_fopen)(const char *pathname, const char *mode) = nullptr;
static int (*orig_openat)(int dirfd, const char *pathname, int flags, mode_t mode) = nullptr;

int my_ptrace(int request, pid_t pid, void *addr, void *data) {
    if (request == PTRACE_ATTACH || request == PTRACE_SEIZE) {
        LOGI(OBFUSCATE("[GOD-TIER] Blocked ACE ptrace attach!"));
        return 0; 
    }
    return orig_ptrace(request, pid, addr, data);
}

FILE* my_fopen(const char *pathname, const char *mode) {
    if (pathname != nullptr) {
        // Block JingMatrix and LSPatch traces
        if (strstr(pathname, OBFUSCATE("liblspd")) || 
            strstr(pathname, OBFUSCATE("lspd")) || 
            strstr(pathname, OBFUSCATE("lspatch")) ||
            strstr(pathname, OBFUSCATE("loader.dex")) ||
            strstr(pathname, OBFUSCATE("libcrypto")) || 
            strstr(pathname, OBFUSCATE("libsys_check"))) {
            return nullptr;
        }
    }
    return orig_fopen(pathname, mode);
}

int my_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
    if (pathname != nullptr) {
        if (strstr(pathname, OBFUSCATE("liblspd")) || 
            strstr(pathname, OBFUSCATE("lspd")) || 
            strstr(pathname, OBFUSCATE("lspatch")) ||
            strstr(pathname, OBFUSCATE("loader.dex")) ||
            strstr(pathname, OBFUSCATE("libcrypto")) ||
            strstr(pathname, OBFUSCATE("magisk")) ||
            strstr(pathname, OBFUSCATE("frida"))) {
            return -1;
        }
    }
    return orig_openat(dirfd, pathname, flags, mode);
}

// =====================================================================
// 2. JINGMATRIX / LSPATCH NUKER
// =====================================================================
static void nukeLSPatchFiles() {
    // JingMatrix extracts loader to these dirs
    const char* paths[] = {
        "/data/data/com.dts.freefiremax/cache",
        "/data/data/com.dts.freefiremax/files",
        "/data/data/com.dts.freefiremax/code_cache"
    };
    
    for (int i = 0; i < 3; i++) {
        DIR* d = opendir(paths[i]);
        if (d) {
            struct dirent* e;
            while ((e = readdir(d)) != NULL) {
                if (strstr(e->d_name, OBFUSCATE("lspatch")) || 
                    strstr(e->d_name, OBFUSCATE("lspd")) || 
                    strstr(e->d_name, OBFUSCATE("loader"))) {
                    char full_path[256];
                    snprintf(full_path, sizeof(full_path), "%s/%s", paths[i], e->d_name);
                    unlink(full_path);
                    LOGI(OBFUSCATE("[LSPATCH-NUKER] Deleted: %s"), full_path);
                }
            }
            closedir(d);
        }
    }
}

// =====================================================================
// 3. ACE THREAD FREEZER
// =====================================================================
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

// =====================================================================
// 4. WATCHDOG
// =====================================================================
static void* guard_thread(void*) {
    LOGI(OBFUSCATE("[GOD-TIER] JingMatrix bypass active."));
    nukeLSPatchFiles();

    void* handle = dlopen("libc.so", RTLD_NOW);
    if (handle) {
        void* ptrace_sym = dlsym(handle, OBFUSCATE("ptrace"));
        void* fopen_sym = dlsym(handle, OBFUSCATE("fopen"));
        void* openat_sym = dlsym(handle, OBFUSCATE("openat"));
        
        if (ptrace_sym) DobbyHook(ptrace_sym, (void*)my_ptrace, (void**)&orig_ptrace);
        if (fopen_sym)  DobbyHook(fopen_sym, (void*)my_fopen, (void**)&orig_fopen);
        if (openat_sym) DobbyHook(openat_sym, (void*)my_openat, (void**)&orig_openat);
        
        dlclose(handle);
    }

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
