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
#include "Dobby/dobby.h" // Included via Macros.h usually, but added here for safety

// ============================================================================
// 1. DOBBY HOOKS - SYSCALL INTERCEPTION
// ============================================================================

// Original function pointers
static int (*orig_openat)(int dirfd, const char *pathname, int flags, mode_t mode) = nullptr;
static int (*orig_stat)(const char *pathname, struct stat *statbuf) = nullptr;
static int (*orig___statx)(int dirfd, const char *pathname, int flags, unsigned int mask, struct statx *statxbuf) = nullptr;

// Hook function for openat - blocks ACE from reading our lib
int my_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
    if (pathname != nullptr) {
        // Block ACE from finding magisk/frida/lsposed
        if (strstr(pathname, OBFUSCATE("magisk")) || 
            strstr(pathname, OBFUSCATE("frida")) || 
            strstr(pathname, OBFUSCATE("lsposed")) ||
            strstr(pathname, OBFUSCATE("xposed"))) {
            return -1; // File not found
        }
        // Block ACE from reading our specific payload name if extracted
        if (strstr(pathname, OBFUSCATE("libsys_check.dat")) ||
            strstr(pathname, OBFUSCATE("libcrypto.so"))) {
            // If ACE tries to read it directly, deny access
            // (Note: System.load needs to read it once, we let system loaders pass)
            if (strstr(pathname, OBFUSCATE("/data/app/")) == nullptr) {
                return -1; 
            }
        }
    }
    return orig_openat(dirfd, pathname, flags, mode);
}

// Hook function for stat - hides files from directory listings
int my_stat(const char *pathname, struct stat *statbuf) {
    if (pathname != nullptr) {
        if (strstr(pathname, OBFUSCATE("magisk")) || 
            strstr(pathname, OBFUSCATE("frida")) || 
            strstr(pathname, OBFUSCATE("lsposed"))) {
            return -1;
        }
    }
    return orig_stat(pathname, statbuf);
}

// ============================================================================
// 2. ANTI-DEBUG & TRACER CHECKS
// ============================================================================

static void killSelf() {
    LOGI(OBFUSCATE("[ACE-SLAYER] Integrity fail -- self-terminating"));
    // Instead of SIGKILL, we use _exit to avoid crash handlers catching it
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
    // If ptrace fails, someone is already tracing us
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0) {
        return true; // Traced
    }
    return false;
}

// ============================================================================
// 3. MEMORY SCANS (FRIDA / MAGISK)
// ============================================================================

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
            strstr(line, OBFUSCATE("gdbserver")) ||
            strstr(line, OBFUSCATE("magisk"))) {
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

static bool checkFridaPorts() {
    FILE* f = fopen(OBFUSCATE("/proc/net/tcp"), "r");
    if (!f) return false;
    char line[512];
    bool found = false;
    while (fgets(line, sizeof(line), f)) {
        // 27042 (69A2), 27043 (69A3), 8700 JDWP (21FC)
        if (strstr(line, OBFUSCATE(":69A2")) ||   
            strstr(line, OBFUSCATE(":69A3")) ||   
            strstr(line, OBFUSCATE(":21FC"))) {   
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

// ============================================================================
// 4. GOD TIER ACE THREAD FREEZER
// ============================================================================

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
                // Check for ACE, TenProtect, Terrors, Beacon, etc.
                if (strstr(comm, OBFUSCATE("ACE")) || 
                    strstr(comm, OBFUSCATE("ace")) || 
                    strstr(comm, OBFUSCATE("terrors")) || 
                    strstr(comm, OBFUSCATE("Beacon")) ||
                    strstr(comm, OBFUSCATE("TP")) ||
                    strstr(comm, OBFUSCATE("mrpcs"))) {
                    
                    int tid = atoi(e->d_name);
                    // Send SIGSTOP to freeze the thread. 
                    // The game thread runs, ACE thread sleeps.
                    syscall(SYS_tgkill, getpid(), tid, SIGSTOP);
                    LOGI(OBFUSCATE("[ACE-SLAYER] Suspended Thread: %s"), comm);
                }
            }
            fclose(f);
        }
    }
    closedir(d);
}

// ============================================================================
// 5. WATCHDOG THREAD
// ============================================================================

static void* guard_thread(void*) {
    LOGI(OBFUSCATE("[ACE-SLAYER] Watchdog started."));
    
    // Wait for game to load before freezing
    sleep(5); 

    // Apply Dobby Hooks
    void* handle = dlopen("libc.so", RTLD_NOW);
    if (handle) {
        void* openat_sym = dlsym(handle, OBFUSCATE("openat"));
        void* stat_sym = dlsym(handle, OBFUSCATE("stat"));
        
        if (openat_sym) {
            DobbyHook(openat_sym, (void*)my_openat, (void**)&orig_openat);
            LOGI(OBFUSCATE("[ACE-SLAYER] Hooked openat."));
        }
        if (stat_sym) {
            DobbyHook(stat_sym, (void*)my_stat, (void**)&orig_stat);
            LOGI(OBFUSCATE("[ACE-SLAYER] Hooked stat."));
        }
        dlclose(handle);
    }

    while (true) {
        // 1. Anti-Debug Checks
        if (checkTracerPidSelf()) { killSelf(); }
        if (checkPtrace())        { killSelf(); }
        
        // 2. Frida / Magisk Checks
        if (checkFridaMaps())     { killSelf(); }
        if (checkFridaPorts())    { killSelf(); }

        // 3. The God-Tier Freeze
        // Every 2 seconds, hunt down ACE and put it to sleep
        suspendACEThreads(); 

        sleep(2); // Fast loop for freeze
    }
    return nullptr;
}

__attribute__((constructor))
void guard_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}
