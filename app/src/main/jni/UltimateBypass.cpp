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
#include <stdarg.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Dobby/dobby.h"

// =====================================================================
// 1. DOBBY HOOKS - KERNEL SYSCALL INTERCEPTION (GOD TIER)
// =====================================================================

static int (*orig_ptrace)(int request, pid_t pid, void *addr, void *data) = nullptr;
static FILE* (*orig_fopen)(const char *pathname, const char *mode) = nullptr;
static int (*orig_openat)(int dirfd, const char *pathname, int flags, mode_t mode) = nullptr;

// Hook ptrace to block ACE from attaching to our memory
int my_ptrace(int request, pid_t pid, void *addr, void *data) {
    if (request == PTRACE_ATTACH || request == PTRACE_SEIZE) {
        // LIE TO ACE: Tell it the attach was successful, but don't do it.
        LOGI(OBFUSCATE("[GOD-TIER] Blocked ACE ptrace attach!"));
        return 0; 
    }
    return orig_ptrace(request, pid, addr, data);
}

// Hook fopen to hide our files and LSPatch traces
FILE* my_fopen(const char *pathname, const char *mode) {
    if (pathname != nullptr) {
        // If ACE tries to read its own maps or our lib, return NULL (File not found)
        if (strstr(pathname, OBFUSCATE("libcrypto")) || 
            strstr(pathname, OBFUSCATE("libsys_check")) ||
            strstr(pathname, OBFUSCATE("lspatch"))) {
            return nullptr;
        }
    }
    return orig_fopen(pathname, mode);
}

// Hook openat to hide our LSPatch files
int my_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
    if (pathname != nullptr) {
        if (strstr(pathname, OBFUSCATE("lspatch")) || 
            strstr(pathname, OBFUSCATE("libcrypto")) ||
            strstr(pathname, OBFUSCATE("magisk")) ||
            strstr(pathname, OBFUSCATE("frida"))) {
            return -1; // Access Denied / File Not Found
        }
    }
    return orig_openat(dirfd, pathname, flags, mode);
}

// =====================================================================
// 2. LSPATCH NUKER (DELETE THE EVIDENCE)
// =====================================================================

static void nukeLSPatchFiles() {
    // Delete the LSPatch cache and loader files from the app's private directory
    DIR* d = opendir(OBFUSCATE("/data/data/com.dts.freefiremax/cache"));
    if (d) {
        struct dirent* e;
        while ((e = readdir(d)) != NULL) {
            if (strstr(e->d_name, OBFUSCATE("lspatch")) || strstr(e->d_name, OBFUSCATE("loader"))) {
                char path[256];
                snprintf(path, sizeof(path), OBFUSCATE("/data/data/com.dts.freefiremax/cache/%s"), e->d_name);
                unlink(path); // DELETE FROM DISK
                LOGI(OBFUSCATE("[LSPATCH-NUKER] Deleted: %s"), path);
            }
        }
        closedir(d);
    }
}

// =====================================================================
// 3. ANTI-DEBUG & THREAD FREEZER
// =====================================================================

static void killSelf() {
    _exit(0); // Hard exit, bypass crash handlers
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
                if (strstr(comm, OBFUSCATE("ACE")) || 
                    strstr(comm, OBFUSCATE("terrors")) || 
                    strstr(comm, OBFUSCATE("TP"))) {
                    int tid = atoi(e->d_name);
                    syscall(SYS_tgkill, getpid(), tid, SIGSTOP); // Freeze ACE
                }
            }
            fclose(f);
        }
    }
    closedir(d);
}

// =====================================================================
// 4. WATCHDOG & HOOK INITIALIZATION
// =====================================================================

static void* guard_thread(void*) {
    LOGI(OBFUSCATE("[GOD-TIER] Bypass active."));
    
    // Nuke LSPatch before ACE loads
    nukeLSPatchFiles();

    // Apply Dobby Hooks
    void* handle = dlopen("libc.so", RTLD_NOW);
    if (handle) {
        void* ptrace_sym = dlsym(handle, OBFUSCATE("ptrace"));
        void* fopen_sym = dlsym(handle, OBFUSCATE("fopen"));
        void* openat_sym = dlsym(handle, OBFUSCATE("openat"));
        
        if (ptrace_sym) DobbyHook(ptrace_sym, (void*)my_ptrace, (void**)&orig_ptrace);
        if (fopen_sym)  DobbyHook(fopen_sym, (void*)my_fopen, (void**)&orig_fopen);
        if (openat_sym) DobbyHook(openat_sym, (void*)my_openat, (void**)&orig_openat);
        
        LOGI(OBFUSCATE("[GOD-TIER] All syscalls hooked."));
        dlclose(handle);
    }

    while (true) {
        // Aggressive ACE Freezer
        suspendACEThreads();
        sleep(1); // Loop every 1 second
    }
    return nullptr;
}

__attribute__((constructor))
void bypass_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}
